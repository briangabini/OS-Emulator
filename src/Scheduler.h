#pragma once

#include <map>
#include "Process.h"

class Process;

class Scheduler {
public:
    virtual ~Scheduler() = default;
    virtual void addProcess(Process* process) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual bool isRunning() const = 0;
    virtual bool isPaused() const = 0;

    virtual int getTotalCores() const = 0;
    virtual int getBusyCores() const = 0;

    virtual std::map<Process*, int> getRunningProcesses() const = 0;
};
