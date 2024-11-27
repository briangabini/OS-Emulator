#include "AScheduler.h"
#include "ConsoleManager.h"
#include "MemoryManager.h"
#include "GlobalConfig.h"
#include "GlobalScheduler.h"
#include "os_emu_vs.h"
#include "SchedulerWorker.h"
#include <iostream>

void SchedulerWorker::run() {
	//std::cout << "SchedulerWorker #" << this->cpuCoreId << " Waiting... " << "running : " << running << std::endl;

	// sleep thread for 5 seconds
	//std::this_thread::sleep_for(std::chrono::seconds(5));

	while (true) {
		std::shared_ptr<Process> process;

		{
			//std::cout << "Worker " << cpuCoreId << " is waiting" << std::endl;

			std::unique_lock<std::mutex> lock(scheduler->queueMutex);

			// wait when there is no process in the queue
			scheduler->queueCV.wait(lock, [this] { return !scheduler->readyQueue.empty() || !scheduler->running; }); // not empty, or not running

			//std::cout << "Worker " << cpuCoreId << " is running" << std::endl;

			if (!scheduler->running && scheduler->readyQueue.empty()) {
				return; // Exit if the scheduler stops running and there's no work left
			}

			if (!scheduler->readyQueue.empty()) {
				//std::cout << "ready queue not empty, size: " << scheduler->readyQueue.size() << std::endl;
				process = scheduler->readyQueue.front();

				//std::cout << "CPU COre ID: " << cpuCoreId << " Process Name: " << process->getName() << " Process State: " << process->getState() << std::endl;

				auto memory = MemoryManager::getInstance()->getMemoryAllocator();
				void* allocatedMemory = memory->allocate(process);

				if (allocatedMemory != nullptr) {
					scheduler->readyQueue.pop();
					this->update(true);
					scheduler->incrementActiveWorkers();
				}
				else {
					//std::cout << "CPU Core ID: " << this->cpuCoreId << " Memory allocation failed for process: " << process->getName() << std::endl;

					// add to the back of the queue
					//scheduler->readyQueue.pop();
					//scheduler->readyQueue.push(process);
					//std::cout << "allocatedMemory is nullptr";
					continue;
				}

			}
		}



		// Process execution
		if (process) {
			//currentProcess->setMemoryPtr(allocatedMemory);
			process->setState(Process::RUNNING);
			process->setCpuCoreId(cpuCoreId);

			SchedulingAlgorithm algo = scheduler->getSchedulingAlgo();

			int execDelay = GlobalConfig::getInstance()->getDelayPerExec();
			int quantumCycle = GlobalConfig::getInstance()->getQuantumCycles();

			int endExecDelay;
			int endCpuCycle;

			if (algo == SchedulingAlgorithm::FCFS)
			{
				while (process->getState() == Process::RUNNING && running) {
					process->setState(Process::RUNNING);
					process->executeCurrentCommand();
					process->moveToNextLine();

					endExecDelay = GlobalScheduler::getInstance()->getCpuCycles() + execDelay;
					while (GlobalScheduler::getInstance()->getCpuCycles() % (endExecDelay + 1) != 0)
					{
						// Busy waiting
					}
				}
				GlobalScheduler::getInstance()->logMemory();
			}
			else if (algo == SchedulingAlgorithm::ROUND_ROBIN)
			{
				endCpuCycle = GlobalScheduler::getInstance()->getCpuCycles() + quantumCycle;

				while (process->getState() == Process::RUNNING && running)
				{
					process->setState(Process::RUNNING);
					process->executeCurrentCommand();
					process->moveToNextLine();

					endExecDelay = GlobalScheduler::getInstance()->getCpuCycles() + execDelay;
					while (GlobalScheduler::getInstance()->getCpuCycles() % (endExecDelay + 1) != 0)
					{
						// Busy waiting
					}

					if (GlobalScheduler::getInstance()->getCpuCycles() >= endCpuCycle)
					{
						if (!process->isFinished()) {
							GlobalScheduler::getInstance()->logMemory();
						}
						break;
					}
				}

				if (!process->isFinished())
				{
					std::unique_lock<std::mutex> lock2(scheduler->memoryMutex);
					process->setState(Process::READY);
					MemoryManager::getInstance()->getMemoryAllocator()->deallocate(process);
					scheduler->readyQueue.push(process);
				}
			}


			if (process->isFinished()) {
				std::unique_lock<std::mutex> lock2(scheduler->memoryMutex);
				MemoryManager::getInstance()->getMemoryAllocator()->deallocate(process);
				process->setState(Process::FINISHED);

				ConsoleManager::getInstance()->unregisterScreen(process->getName());

				// do some logging
				GlobalScheduler::getInstance()->logMemory();
			} else if (process->getMemoryPtr() != nullptr) {
				std::unique_lock<std::mutex> lock2(scheduler->memoryMutex);
				MemoryManager::getInstance()->getMemoryAllocator()->deallocate(process);
			}

			this->update(false); // Mark the worker as free
			scheduler->decrementActiveWorkers();
		}
	}
}


void SchedulerWorker::update(bool running) {
	this->running = running;
}