#ifndef CHEESYD_WORKFLOW_H_
#define CHEESYD_WORKFLOW_H_

#include <iostream>
#include <memory>

#include <hiredis/hiredis.h>
#include <json11/json11.hpp>

namespace cheesyd {

std::string const JOB_STATUS_REQUESTED = "requested";
std::string const JOB_STATUS_IN_PROGRESS = "in_progress";
std::string const JOB_STATUS_DONE = "done";
std::string const JOB_STATUS_ERROR = "error";

class JobNotFoundException : public std::exception {
private:
    std::string m_what;
public:
    JobNotFoundException(std::string t_what);

    const char *what();
};

class JobData {
public:
    JobData(std::string t_payload_str, std::string t_status);

    json11::Json payload;
    std::string status;
};

class Workflow {
private:
    redisContext *m_redis_ctx;

    Workflow(redisContext *t_redis_ctx);

public:
    static std::unique_ptr<Workflow> Create();

    Workflow(const Workflow &) = delete;

    ~Workflow();

    Workflow &operator=(const Workflow &) = delete;

    std::string DequeueJob();

    JobData GetJobData(std::string job_id);
};

}

#endif // CHEESYD_WORKFLOW_H_