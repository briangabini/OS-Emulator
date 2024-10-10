#include "FCFSScheduler.h"
#include "SchedulerWorker.h"
#include <iostream>

FCFSScheduler::FCFSScheduler()
	: AScheduler(SchedulingAlgorithm::FCFS) {
}

void FCFSScheduler::init() {
	//for (auto& process : processes) {
	//	readyQueue.push(process);
	//}
}

std::shared_ptr<SchedulerWorker> FCFSScheduler::findAvailableWorker() {
	for (auto& worker : schedulerWorkers) {
		if (!worker->isRunning()) {
			return worker;
		}
	}
	return nullptr;
}

void FCFSScheduler::execute() {
	while (!readyQueue.empty()) {
		std::shared_ptr<Process> currentProcess = readyQueue.front();
		readyQueue.pop();

		std::shared_ptr<SchedulerWorker> worker = nullptr;

		while (!(worker = findAvailableWorker())) {
			sleep(200);
		}
		worker->update(true);
		worker->assignProcess(currentProcess);
	}
}