#include "workflow.hpp"

#include <string.h>
#include <iostream>
#include <memory>

#include <hiredis/hiredis.h>

namespace cheesyd {
Workflow::Workflow(redisContext *t_redis_ctx) : m_redis_ctx(t_redis_ctx) {
    std::cout << "Workflow is ready with a redis connection\n";
}

Workflow::~Workflow() {
    redisFree(m_redis_ctx);
    std::cout << "Workflow is gone and the redis connection is free\n";
}

std::unique_ptr<Workflow> Workflow::Create() {
    const char *hostname = "127.0.0.1";
    int port = 6379;
    struct timeval timeout = {1, 500000}; // 1.5 seconds
    auto redis_ctx = redisConnectWithTimeout(hostname, port, timeout);

    if (redis_ctx == NULL || redis_ctx->err) {
        if (redis_ctx) {
            std::cout << "Connection error: " << redis_ctx->errstr << "\n";
            redisFree(redis_ctx);
        } else {
            std::cout << "Connection error: can't allocate redis context\n";
        }

        return std::unique_ptr<Workflow>(nullptr);
    }

    // PING server
    auto reply = (redisReply *) redisCommand(redis_ctx, "PING");
    bool is_connection_ok = strcmp("PONG", reply->str) == 0;
    std::cout << "Testing redis connection: PING -> " << reply->str << " [" << (is_connection_ok ? "ok" : "no") << "]\n";
    freeReplyObject(reply);

    if (is_connection_ok) {
        return std::unique_ptr<Workflow>(new Workflow(redis_ctx));
    } else {
        redisFree(redis_ctx);
        return std::unique_ptr<Workflow>(nullptr);
    }
}

std::string Workflow::DequeueJob() {
    auto reply = (redisReply *) redisCommand(m_redis_ctx, "RPOPLPUSH cheesyd:queue:job_request cheesyd:queue:job_in_progress");
    if (reply->str) {
        std::string reply_content(reply->str);
        return reply_content;
    }

    return "";
}
}