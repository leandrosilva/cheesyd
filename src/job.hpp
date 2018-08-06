#ifndef CHEESYD_WORKFLOW_H
#define CHEESYD_WORKFLOW_H

#include <iostream>
#include <memory>

#include <hiredis/hiredis.h>
#include <json11/json11.hpp>

namespace cheesyd {

std::string const JOB_STATUS_REQUESTED = "requested";
std::string const JOB_STATUS_IN_PROGRESS = "in_progress";
std::string const JOB_STATUS_DONE = "done";
std::string const JOB_STATUS_ERROR = "error";

class GenericException : public std::exception {
private:
    std::string m_what;
public:
    GenericException(std::string t_what);

    virtual const char *what();
};

class InvalidJobException : public GenericException {
public:
    InvalidJobException(std::string t_what);
};

class JobData {
public:
    JobData(std::string t_payload_str, std::string t_status);
    JobData(std::string t_payload_str, std::string t_result, std::string t_status);

    json11::Json payload;
    std::string result;
    std::string status;
};

class JobManager {
private:
    redisContext *m_redis_ctx;

    JobManager(redisContext *t_redis_ctx);

public:
    static std::unique_ptr<JobManager> Create();

    JobManager(const JobManager &) = delete;
    ~JobManager();

    JobManager &operator=(const JobManager &) = delete;

    std::string DequeueJob();
    JobData GetJobData(std::string job_id);
    void StoreJobResult(std::string job_id, const unsigned char *pdf_content, unsigned long pdf_content_length);
    void FinishJob(std::string job_id);
    void FinishJob(std::string job_id, std::string error_message);
};

}

#endif // CHEESYD_WORKFLOW_H