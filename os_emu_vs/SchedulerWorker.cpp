#include "AScheduler.h"
#include "ConsoleManager.h"
#include "GlobalConfig.h"
#include "os_emu_vs.h"
#include "SchedulerWorker.h"
#include <iostream>

void SchedulerWorker::run() {
    while (true) {
        std::shared_ptr<Process> process;

        {
            std::unique_lock<std::mutex> lock(scheduler->queueMutex);

            scheduler->queueCV.wait(lock, [this] {
                return !scheduler->readyQueue.empty() || !scheduler->running;
                });

            if (!scheduler->running && scheduler->readyQueue.empty()) {
                return;
            }

            if (!scheduler->readyQueue.empty()) {
                process = scheduler->readyQueue.front();

                // Skip if process is already finished
                if (process->getState() == Process::FINISHED) {
                    scheduler->readyQueue.pop();
                    continue;
                }

                scheduler->readyQueue.pop();
                this->update(true);
                scheduler->incrementActiveWorkers();
            }
        }

        if (process) {
            bool shouldRequeueProcess = false;
            bool isProcessFinished = false;  // Declare process state flags here

            // Try to allocate memory if needed
            if (!process->hasMemory()) {
                try {
                    bool allocated = process->allocateMemory();
                    if (!allocated) {
                        shouldRequeueProcess = true;
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Memory allocation failed for process " << process->getName()
                        << ": " << e.what() << std::endl;
                    shouldRequeueProcess = true;
                }
            }

            if (!shouldRequeueProcess) {
                try {
                    process->setState(Process::RUNNING);
                    process->setCpuCoreId(cpuCoreId);

                    SchedulingAlgorithm algo = scheduler->getSchedulingAlgo();
                    int execDelay = GlobalConfig::getInstance()->getDelayPerExec();
                    int quantumCycle = GlobalConfig::getInstance()->getQuantumCycles();

                    if (algo == SchedulingAlgorithm::FCFS) {
                        while (!process->isFinished() && running) {
                            process->executeCurrentCommand();
                            process->moveToNextLine();

                            int endExecDelay = cpuCycles + execDelay;
                            while (cpuCycles < endExecDelay) {
                                // Just wait
                            }
                        }
                        isProcessFinished = process->isFinished();
                    }
                    else if (algo == SchedulingAlgorithm::ROUND_ROBIN) {
                        int endCpuCycle = cpuCycles + quantumCycle;

                        while (!process->isFinished() && running && cpuCycles < endCpuCycle) {
                            process->executeCurrentCommand();
                            process->moveToNextLine();

                            int endExecDelay = cpuCycles + execDelay;
                            while (cpuCycles < endExecDelay) {
                                // Just wait
                            }
                        }
                        isProcessFinished = process->isFinished();
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Error executing process " << process->getName()
                        << ": " << e.what() << std::endl;
                    isProcessFinished = true; // Mark as finished if we encounter an error
                }
            }

            // Handle process state changes
            {
                std::unique_lock<std::mutex> lock(scheduler->queueMutex);

                try {
                    if (isProcessFinished) {
                        process->setState(Process::FINISHED);
                    }
                    else {
                        process->setState(Process::READY);
                        if (shouldRequeueProcess || !isProcessFinished) {
                            scheduler->readyQueue.push(process);
                        }
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Error updating process state for " << process->getName()
                        << ": " << e.what() << std::endl;
                }
            }

            this->update(false);
            scheduler->decrementActiveWorkers();
        }
    }
}

void SchedulerWorker::update(bool isRunning) {
    this->running = isRunning;
}