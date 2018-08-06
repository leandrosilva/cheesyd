#include "job.hpp"
#include "base64.hpp"

#include <string.h>
#include <iostream>
#include <memory>
#include <fstream>

#include <hiredis/hiredis.h>
#include <json11/json11.hpp>

namespace cheesyd {

GenericException::GenericException(std::string t_what) : m_what(t_what) {
}

const char *GenericException::what() {
    return m_what.c_str();
}

InvalidJobException::InvalidJobException(std::string t_what) : GenericException(t_what) {
}

JobData::JobData(std::string t_payload_str, std::string t_status) {
    std::string parsing_err;
    payload = json11::Json::parse(t_payload_str, parsing_err);
    result = "";
    status = t_status;
}

JobData::JobData(std::string t_payload_str, std::string t_result, std::string t_status) : JobData(t_payload_str, t_status) {
    result = t_result;
}

JobManager::JobManager(redisContext *t_redis_ctx) : m_redis_ctx(t_redis_ctx) {
    std::cout << "JobManager is ready with a redis connection\n";
}

JobManager::~JobManager() {
    redisFree(m_redis_ctx);
    std::cout << "JobManager is gone and the redis connection is free" << std::endl;
}

std::unique_ptr<JobManager> JobManager::Create() {
    const char *hostname = "127.0.0.1";
    const int port = 6379;
    const struct timeval timeout = {1, 500000}; // 1.5 seconds
    auto redis_ctx = redisConnectWithTimeout(hostname, port, timeout);

    if (redis_ctx == NULL || redis_ctx->err) {
        if (redis_ctx) {
            std::cout << "Connection error: " << redis_ctx->errstr << "\n";
            redisFree(redis_ctx);
        } else {
            std::cout << "Connection error: can't allocate redis context\n";
        }

        return std::unique_ptr<JobManager>(nullptr);
    }

    // PING server
    auto reply = (redisReply *) redisCommand(redis_ctx, "PING");
    bool is_connection_ok = strcmp("PONG", reply->str) == 0;
    std::cout << "Testing redis connection: PING -> " << reply->str << " [" << (is_connection_ok ? "ok" : "no") << "]\n";
    freeReplyObject(reply);

    if (is_connection_ok) {
        return std::unique_ptr<JobManager>(new JobManager(redis_ctx));
    } else {
        redisFree(redis_ctx);
        return std::unique_ptr<JobManager>(nullptr);
    }
}

std::string JobManager::DequeueJob() {
    auto reply = (redisReply *) redisCommand(m_redis_ctx, "RPOPLPUSH cheesyd:queue:job_request cheesyd:queue:job_in_progress");
    if (reply->str) {
        std::string reply_content(reply->str);
        freeReplyObject(reply);
        return reply_content;
    }

    return "";
}

JobData JobManager::GetJobData(std::string job_id) {
    auto reply = (redisReply *) redisCommand(m_redis_ctx, "HGETALL cheesyd:job:%s", job_id.c_str());
    if (reply->elements) {
        std::map<std::string, std::string> fields;
        for (size_t i = 0; i < reply->elements; i++) {
            if (((i + 1) % 2) == 0) {
                std::string key(reply->element[i - 1]->str);
                std::string value(reply->element[i]->str);
                fields[key] = value;
            }
        }
        freeReplyObject(reply);

        if (fields["payload"].empty()) {
            throw InvalidJobException("Job ID " + job_id + " does not provide payload field");
        }
        return JobData(fields["payload"], fields["result"], fields["status"]);
    }

    throw InvalidJobException("Job ID " + job_id + " could not be found (no data available)");
}

void JobManager::StoreJobResult(std::string job_id, const unsigned char *pdf_content, unsigned long pdf_content_length) {
    std::string encode = base64_encode(pdf_content, pdf_content_length);
    auto reply = (redisReply *) redisCommand(m_redis_ctx, "HSET cheesyd:job:%s result %s", job_id.c_str(), encode.c_str());
    if (reply->type != REDIS_REPLY_ERROR) {
        std::cout << "PDF content store on redis\n";

        // Just for test purpose while developing
        auto job_data = GetJobData(job_id);
        if (!job_data.result.empty()) {
            std::cout << "PDF size: local (" << encode.size() << " bytes) == redis (" << job_data.result.size() << " bytes)\n";
            std::string decode = base64_decode(job_data.result);
            const auto *content = (const unsigned char *) decode.c_str();
            std::string pdf_file_name("test-" + job_id + ".pdf");
            std::ofstream pdf_file(pdf_file_name, std::ios::binary);
            for (unsigned int i = 0; i < pdf_content_length; i++) pdf_file << std::noskipws << content[i];
            pdf_file.close();
            std::cout << "PDF saved to " << pdf_file_name << "\n";
        }
    }
    freeReplyObject(reply);
}

void JobManager::FinishJob(std::string job_id) {
    auto reply = (redisReply *) redisCommand(m_redis_ctx, "HSET cheesyd:job:%s status done", job_id.c_str());
    if (reply->type != REDIS_REPLY_ERROR) {
        std::cout << "Job " << job_id << " is tagged <done>\n";

        reply = (redisReply *) redisCommand(m_redis_ctx, "LREM cheesyd:queue:job_in_progress 0 %s", job_id.c_str());
        if (reply->type != REDIS_REPLY_ERROR) {
            std::cout << "Job " << job_id << " was removed from the in progress queue\n";
        }

        reply = (redisReply *) redisCommand(m_redis_ctx, "LPUSH cheesyd:queue:job_done %s", job_id.c_str());
        if (reply->type != REDIS_REPLY_ERROR) {
            std::cout << "Job " << job_id << " was pushed to the done queue\n";
        }
    }
    freeReplyObject(reply);
}

void JobManager::FinishJob(std::string job_id, std::string error_message) {
    auto reply = (redisReply *) redisCommand(m_redis_ctx, "HSET cheesyd:job:%s status error error %s", job_id.c_str(), error_message.c_str());
    if (reply->type != REDIS_REPLY_ERROR) {
        std::cout << "Job " << job_id << " is tagged <error>\n";

        reply = (redisReply *) redisCommand(m_redis_ctx, "LREM cheesyd:queue:job_in_progress 0 %s", job_id.c_str());
        if (reply->type != REDIS_REPLY_ERROR) {
            std::cout << "Job " << job_id << " was removed from the in progress queue\n";
        }

        reply = (redisReply *) redisCommand(m_redis_ctx, "LPUSH cheesyd:queue:job_error %s", job_id.c_str());
        if (reply->type != REDIS_REPLY_ERROR) {
            std::cout << "Job " << job_id << " was pushed to the error queue\n";
        }
    }
    freeReplyObject(reply);
}

}