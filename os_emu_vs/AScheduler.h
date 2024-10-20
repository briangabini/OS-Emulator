#pragma once
#include "IETThread.h"
#include "Process.h"
#include "SchedulerWorker.h"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

static const std::string DEBUG_SCHEDULER_NAME = "DebugScheduler";
static const std::string FCFS_SCHEDULER_NAME = "FCFSScheduler";
static const std::string SJF_NOPREEMPT_SCHEDULER_NAME = "SJF-NoPreempt-Scheduler";
static const std::string SJF_PREEMPT_SCHEDULER_NAME = "SJF-Preempt-Scheduler";

class SchedulerWorker;

class AScheduler : public IETThread {
public:
	enum SchedulingAlgorithm {
		FCFS,
		ROUND_ROBIN
		// DEBUG,
		// SHORTEST_JOB_FIRST_NONPREEMPTIVE,
		// SHORTEST_JOB_FIRST_PREEMPTIVE,
	};

	AScheduler(SchedulingAlgorithm schedulingAlgo);
	virtual ~AScheduler() = default;

	void addProcess(std::shared_ptr<Process> process);
	void incrementActiveWorkers();
	void decrementActiveWorkers();
	int getActiveWorkersCount() const;
	double getCpuUtilization() const;
	void run() override;

	virtual void init() = 0;
	virtual void execute() = 0;

	friend class GlobalScheduler;


protected:
	SchedulingAlgorithm schedulingAlgo;
	bool running = true;
	int workersCount = 4;
	std::queue<std::shared_ptr<Process>> readyQueue;
	std::vector<std::shared_ptr<SchedulerWorker>> schedulerWorkers;
	std::mutex queueMutex;
	std::condition_variable queueCV;
	int activeWorkers = 0;

	friend class SchedulerWorker;
};
