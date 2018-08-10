//
// Created by leand on 06/08/18.
//

#include <string>
#include <iostream>

#include <hiredis/hiredis.h>

std::string is_ok(redisReply *reply) {
    return reply->type == REDIS_REPLY_ERROR ? "error" : "ok";
}

void log(const std::string &command, redisReply *reply) {
    std::cout << command << " [" << is_ok(reply) << "]\n";
}

void execute_command(redisContext *redis_ctx, const std::string &command) {
    auto reply = (redisReply *) redisCommand(redis_ctx, command.c_str());
    log(command, reply);
    freeReplyObject(reply);
}

int main() {
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

        return -1;
    }

    std::cout << "\nCLEANING\n";

    execute_command(redis_ctx, "DEL cheesyd:queue:job_request");
    execute_command(redis_ctx, "DEL cheesyd:queue:job_in_progres");
    execute_command(redis_ctx, "DEL cheesyd:queue:job_done");
    execute_command(redis_ctx, "DEL cheesyd:queue:job_error");

    for (int i = 1; i <= 4; i++) {
        execute_command(redis_ctx, "DEL cheesyd:job:j" + std::to_string(i));
    }

    std::cout << "\nSEEDING\n";

    // With right data
    for (int i = 1; i <= 2; i++) {
        execute_command(redis_ctx, "HSET cheesyd:job:j" + std::to_string(i) + R"( payload {"page":"sample1.html","windowStatus":"ready"} status request)");
        execute_command(redis_ctx, "LPUSH cheesyd:queue:job_request j" + std::to_string(i));
    }

    // With bad data
    execute_command(redis_ctx, "HSET cheesyd:job:j3 status request");
    execute_command(redis_ctx, "LPUSH cheesyd:queue:job_request j3");
    // NO DATA -> execute_command(redis_ctx, "HSET cheesyd:job:j4 status request");
    execute_command(redis_ctx, "LPUSH cheesyd:queue:job_request j4");

    redisFree(redis_ctx);
    std::cout << std::endl;

    return 0;
}