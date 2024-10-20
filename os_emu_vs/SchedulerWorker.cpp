#include "SchedulerWorker.h"
#include "AScheduler.h"
#include <iostream>

void SchedulerWorker::update(bool isRunning) {
    this->running = isRunning;
}

void SchedulerWorker::run() {
	//std::cout << "SchedulerWorker #" << this->cpuCoreId << " Waiting... " << "running : " << running << std::endl;

    while (true) {
        std::shared_ptr<Process> process;

        {
			//std::cout << "Worker " << cpuCoreId << " is waiting" << std::endl;

            std::unique_lock<std::mutex> lock(scheduler->queueMutex);

			// wait when there is no process in the queue
			scheduler->queueCV.wait(lock, [this] { return !scheduler->readyQueue.empty(); });

			//std::cout << "Worker " << cpuCoreId << " is running" << std::endl;

            if (!scheduler->running && scheduler->readyQueue.empty()) {
                return; // Exit if the scheduler stops running and there's no work left
            }

            if (!scheduler->readyQueue.empty()) {
				//std::cout << "ready queue not empty, size: " << scheduler->readyQueue.size() << std::endl;
                process = scheduler->readyQueue.front();
                scheduler->readyQueue.pop();
            }
        }

        // Process execution
        if (process) {
            process->setState(Process::RUNNING);
            process->setCpuCoreId(cpuCoreId);

            while (!process->isFinished() && running) {
                process->executeCurrentCommand();
                process->moveToNextLine();
                IETThread::sleep(50);                       // Simulate execution delay
            }

            if (process->isFinished()) {
                process->setState(Process::FINISHED);
            }

            //this->update(false); // Mark the worker as free
        }
    }
}

bool SchedulerWorker::isRunning() {
    return running;
}
