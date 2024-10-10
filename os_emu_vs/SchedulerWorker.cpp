#include "GlobalScheduler.h"
#include "SchedulerWorker.h"
#include <iostream>

void SchedulerWorker::update(bool isRunning) {
	this->running = isRunning;
}

void SchedulerWorker::assignProcess(std::shared_ptr<Process> process) {
	this->currentProcess = process;
	this->update(true);
}

void SchedulerWorker::run() {
	while (true) {
		if (currentProcess) {
			currentProcess->setState(currentProcess->RUNNING);
			currentProcess->setCpuCoreId(cpuCoreId);

			while (!currentProcess->isFinished()) {
				currentProcess->executeCurrentCommand();
				if (currentProcess->getCommandCounter() == currentProcess->getLinesOfCode() - 1) {
					currentProcess->setState(currentProcess->FINISHED);
				}
				currentProcess->moveToNextLine();
				sleep(250);
			}
			currentProcess = nullptr;
			running = false;
		}
	}
}


bool SchedulerWorker::isRunning() {
	return running;
}