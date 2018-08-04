#ifndef CHEESYD_WORKFLOW_H_
#define CHEESYD_WORKFLOW_H_

#include <iostream>
#include <memory>

#include <hiredis/hiredis.h>

namespace cheesyd {
class Workflow {
private:
    redisContext *m_redis_ctx;

    Workflow(redisContext *t_redis_ctx);

public:
    static std::unique_ptr<Workflow> Create();

    Workflow(const Workflow &) = delete;

    Workflow &operator=(const Workflow &) = delete;

    std::string DequeueJob();

    ~Workflow();
};
}

#endif // CHEESYD_WORKFLOW_H_