#include "AScheduler.h"
#include "ConsoleManager.h"
#include "FlatMemoryAllocator.h"
#include "GlobalConfig.h"
#include "os_emu_vs.h"
#include "SchedulerWorker.h"
#include <iostream>

void SchedulerWorker::run() {
	//std::cout << "SchedulerWorker #" << this->cpuCoreId << " Waiting... " << "running : " << running << std::endl;

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
				scheduler->readyQueue.pop();
				this->update(true);
				scheduler->incrementActiveWorkers();
			}
		}

		// Process execution
		if (process) {
			void* allocatedMemory = FlatMemoryAllocator::getInstance()->allocate(process->getMemoryRequired());
			if (allocatedMemory == nullptr) {
				// Not enough memory, move process to the tail of the queue
				std::lock_guard<std::mutex> lock(scheduler->queueMutex);
				scheduler->readyQueue.push(process);
				break;
			}

			process->setState(Process::RUNNING);
			process->setCpuCoreId(cpuCoreId);

			SchedulingAlgorithm algo = scheduler->getSchedulingAlgo();

			int execDelay = GlobalConfig::getInstance()->getDelayPerExec();
			int quantumCycle = GlobalConfig::getInstance()->getQuantumCycles();

			int endExecDelay;
			int endCpuCycle;

			if (algo == SchedulingAlgorithm::FCFS)
			{
				while (!process->isFinished() && running) {
					process->executeCurrentCommand();
					process->moveToNextLine();

					// busy waiting
					endExecDelay = cpuCycles + execDelay;
					while (cpuCycles % (endExecDelay + 1) != 0)
					{
					}
				}
			}
			else if (algo == SchedulingAlgorithm::ROUND_ROBIN)
			{
				endCpuCycle = cpuCycles + quantumCycle;

				while (!process->isFinished() && running)
				{
					process->executeCurrentCommand();
					process->moveToNextLine();

					// busy waiting
					endExecDelay = cpuCycles + execDelay;
					while (cpuCycles % (endExecDelay + 1) != 0)
					{

					}

					if (cpuCycles >= endCpuCycle)
					{
						break;
					}
				}
				if (!process->isFinished())
				{
					std::unique_lock<std::mutex> lock(scheduler->queueMutex);
					FlatMemoryAllocator::getInstance()->deallocate(allocatedMemory);
					scheduler->readyQueue.push(process);
				}
			}


			if (process->isFinished()) {
				FlatMemoryAllocator::getInstance()->deallocate(allocatedMemory);
				process->setState(Process::FINISHED);
				ConsoleManager::getInstance()->unregisterScreen(process->getName());
			}

			this->update(false); // Mark the worker as free
			scheduler->decrementActiveWorkers();
		}
	}
}


void SchedulerWorker::update(bool running) {
	this->running = running;
}