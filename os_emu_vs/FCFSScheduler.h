// FCFSScheduler.h
#pragma once
#include <queue>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "AScheduler.h"
#include "Process.h"
#include "SchedulerWorker.h"

class FCFSScheduler : public AScheduler {
public:
    FCFSScheduler();

    void init() override;
    void execute() override;

private:
    std::queue<std::shared_ptr<Process>> readyQueue;
    std::shared_ptr<SchedulerWorker> findAvailableWorker();
};
