#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class Process;

class Scheduler {
public:
    virtual ~Scheduler() = default;
    virtual void addProcess(Process* process) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};
