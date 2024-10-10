#pragma once
#include "AScheduler.h"
#include <queue>

class FCFSScheduler : public AScheduler {
public:
    FCFSScheduler();

    void init() override;
    void execute() override;

private:
    std::shared_ptr<SchedulerWorker> findAvailableWorker();
};