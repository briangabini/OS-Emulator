// FCFSScheduler.h
#pragma once
#include "AScheduler.h"
#include <queue>
#include <mutex>

class FCFSScheduler : public AScheduler {
public:
    FCFSScheduler();

    void init() override;
    void execute() override;

private:
    std::shared_ptr<SchedulerWorker> findAvailableWorker();
    std::mutex queueMutex;
};
