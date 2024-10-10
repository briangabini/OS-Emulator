#pragma once
#include <string>
#include <queue>

#include "IETThread.h"
#include "Process.h"
#include "SchedulerWorker.h"

static const std::string DEBUG_SCHEDULER_NAME = "DebugScheduler";
static const std::string FCFS_SCHEDULER_NAME = "FCFSScheduler";
static const std::string SJF_NOPREEMPT_SCHEDULER_NAME = "SJF-NoPreempt-Scheduler";
static const std::string SJF_PREEMPT_SCHEDULER_NAME = "SJF-Preempt-Scheduler";

class AScheduler : public IETThread {
public:
	enum SchedulingAlgorithm {
		DEBUG,
		FCFS,
		SHORTEST_JOB_FIRST_NONPREEMPTIVE,
		SHOTEST_JOB_FIRST_PREEMPTIVE,
		ROUND_ROBIN
	};

	AScheduler(SchedulingAlgorithm schedulingAlgo);

	void addProcess(std::shared_ptr<Process> process);
	//std::shared_ptr<Process> findProcess(std::string processName);
	void run() override;
	void stop();

	virtual void init() = 0;
	virtual void execute() = 0;

	struct ProcessInfo {
		int pid;
		std::string name;
		typedef std::vector<std::shared_ptr<ICommand>> CommandList;
		CommandList commandList;
		int commandCounter;
		int cpuCoreId;
		Process::ProcessState currentState;
	};

	friend class GlobalScheduler;


protected:
	SchedulingAlgorithm schedulingAlgo;
	bool running = true;
	int workersCount = 4;
	std::vector<std::shared_ptr<Process>> processes;
	std::queue<std::shared_ptr<Process>> readyQueue;
	std::vector<std::shared_ptr<SchedulerWorker>> schedulerWorkers;
};
