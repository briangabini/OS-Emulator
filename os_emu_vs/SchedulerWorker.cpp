#include "SchedulerWorker.h"
#include "GlobalScheduler.h"
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
            while (!currentProcess->isFinished()) {
                currentProcess->executeCurrentCommand();
                currentProcess->moveToNextLine();
            }
            currentProcess = nullptr;
            running = false;
        }
        sleep(200);
    }
}

bool SchedulerWorker::isRunning() {
    return running;
}