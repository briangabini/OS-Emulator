#include "RRScheduler.h"
#include "GlobalConfig.h"
#include <iostream>

RRScheduler::RRScheduler()
    : AScheduler(SchedulingAlgorithm::ROUND_ROBIN) {
}

void RRScheduler::init() {
    quantumCount = 0;
}

void RRScheduler::execute() {
    size_t quantumCycles = GlobalConfig::getInstance()->getQuantumCycles();

    while (running) {
        // Process management section
        {
            std::unique_lock<std::mutex> lock(queueMutex);

            // Check for processes in the ready queue
            if (!readyQueue.empty()) {
                auto process = readyQueue.front();
                readyQueue.pop();

                // Try to allocate memory if process doesn't have it
                if (!process->hasMemory()) {
                    if (!process->allocateMemory()) {
                        // If memory allocation failed, put process back at end of queue
                        readyQueue.push(process);
                        lock.unlock();
                        continue;
                    }
                }

                // Update process state to running
                process->setState(Process::RUNNING);

                // Execute for quantum duration
                size_t cycleCount = 0;
                while (cycleCount < quantumCycles && !process->isFinished() && running) {
                    process->executeCurrentCommand();
                    process->moveToNextLine();
                    cycleCount++;
                }

                // Check process completion
                if (process->isFinished()) {
                    process->setState(Process::FINISHED);
                    process->deallocateMemory();
                }
                else {
                    // If not finished, set back to ready and return to queue
                    process->setState(Process::READY);
                    readyQueue.push(process);
                }
            }
        }

        // Generate memory snapshot at end of quantum
        MemoryManager::getInstance()->generateMemorySnapshot(quantumCount++);

        // Simulate quantum timing
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
}