#include "Command.h"
#include "Config.h"
#include "SchedulerFirstComeFirstServe.h"
#include <algorithm>
#include <iostream>
#include <thread>

SchedulerFirstComeFirstServe::SchedulerFirstComeFirstServe(int numCores, ConsoleManager& manager)
	: numCores(numCores), running(false), paused(false), consoleManager(manager), cpuCycles(0) {

	for (int i = 0; i < numCores; ++i) {
		Worker* worker = new Worker();
		worker->coreId = i;
		workers.push_back(worker);
	}
}

SchedulerFirstComeFirstServe::~SchedulerFirstComeFirstServe() {
	stop();
	for (Worker* worker : workers) {
		delete worker;
	}
}

void SchedulerFirstComeFirstServe::addProcess(Process* process) {
	{
		std::lock_guard<std::mutex> lock(queuedProcessesMutex);
		if (queuedProcessesSet.find(process) == queuedProcessesSet.end()) {
			queuedProcessesSet.insert(process);
			processQueue.push(process);
		}
	}
	{
		std::lock_guard<std::mutex> lock(allProcessesMutex);
		if (std::find(allProcesses.begin(), allProcesses.end(), process) == allProcesses.end()) {
			allProcesses.push_back(process);
		}
	}
}

void SchedulerFirstComeFirstServe::start() {
	if (running.load()) return;
	running.store(true);
	paused.store(false);
	schedulerThread = std::thread(&SchedulerFirstComeFirstServe::schedulerLoop, this);
}

void SchedulerFirstComeFirstServe::stop() {
	if (!running.load()) return;
	running.store(false);
	paused.store(false);
	pauseCV.notify_all();
	processQueue.stop();
	if (schedulerThread.joinable()) {
		schedulerThread.join();
	}

	for (Worker* worker : workers) {
		worker->cv.notify_all();
		if (worker->thread.joinable()) {
			worker->thread.join();
		}
	}
}

void SchedulerFirstComeFirstServe::pause() {
	if (!running.load() || paused.load()) return;
	paused.store(true);
}

void SchedulerFirstComeFirstServe::resume() {
	if (!running.load() || !paused.load()) return;
	paused.store(false);
	pauseCV.notify_all();
}

bool SchedulerFirstComeFirstServe::isRunning() const {
	return running.load();
}

bool SchedulerFirstComeFirstServe::isPaused() const {
	return paused.load();
}

void SchedulerFirstComeFirstServe::schedulerLoop() {
	for (Worker* worker : workers) {
		worker->thread = std::thread(&SchedulerFirstComeFirstServe::workerLoop, this, worker->coreId);
	}

	while (running.load()) {
		// Pause handling
		while (paused.load()) {
			if (!running.load()) return;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			cpuCycles++;
			consoleManager.getMemoryManager().incrementIdleCpuTicks();
		}
		if (!running.load()) break;

		Process* process = nullptr;
		if (processQueue.wait_and_pop(process)) {
			if (!running.load()) break;

			// Remove from queuedProcessesSet
			{
				std::lock_guard<std::mutex> lock(queuedProcessesMutex);
				queuedProcessesSet.erase(process);
			}

			if (!process->isInMemory()) {
				// Process is not in memory, cannot schedule it
				// Try to allocate memory again
				if (!consoleManager.getMemoryManager().allocateMemory(process, process->getMemorySize())) {
					// Requeue the process
					addProcess(process);
					continue;
				}
			}

			// Assign process to an idle worker
			bool assigned = false;
			while (!assigned && running.load()) {
				for (Worker* worker : workers) {
					std::unique_lock<std::mutex> lock(worker->mtx);
					if (!worker->busy.load()) {
						worker->currentProcess = process;
						worker->busy.store(true);
						worker->cv.notify_one();
						assigned = true;
						break;
					}
				}
				if (!assigned) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					cpuCycles++;
					consoleManager.getMemoryManager().incrementIdleCpuTicks();
				}
			}
		}
	}
}

void SchedulerFirstComeFirstServe::workerLoop(int coreId) {
	Worker* worker = workers[coreId];
	Config& config = Config::getInstance();
	unsigned int delayPerExec = config.getDelaysPerExec();

	while (running.load()) {
		// Pause handling
		while (paused.load()) {
			if (!running.load()) return;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			cpuCycles++;
			consoleManager.getMemoryManager().incrementIdleCpuTicks();
		}
		if (!running.load()) break;

		std::unique_lock<std::mutex> lock(worker->mtx);

		// Wait for a process to be assigned
		worker->cv.wait(lock, [worker, this]() {
			return worker->busy.load() || !running.load();
			});

		if (!running.load()) break;

		if (!worker->busy.load() || worker->currentProcess == nullptr) {
			// Spurious wakeup or process was set to nullptr
			continue;
		}

		Process* process = worker->currentProcess;
		lock.unlock();

		// Check if process is in memory before executing
		if (!process->isInMemory()) {
			// Process not in memory, try to allocate
			if (!consoleManager.getMemoryManager().allocateMemory(process, process->getMemorySize())) {
				// Cannot allocate memory, requeue the process
				lock.lock();
				worker->busy.store(false);
				worker->currentProcess = nullptr;
				lock.unlock();

				addProcess(process);
				continue;
			}
		}

		Command* cmd = nullptr;
		while ((cmd = process->getNextCommand()) != nullptr && running.load()) {
			// Check memory status before each instruction
			if (!process->isInMemory()) {
				// Lost memory allocation, need to requeue
				if (!consoleManager.getMemoryManager().allocateMemory(process, process->getMemorySize())) {
					// Put command back and requeue process
					process->addCommand(cmd);
					addProcess(process);

					// Reset worker state
					lock.lock();
					worker->busy.store(false);
					worker->currentProcess = nullptr;
					lock.unlock();

					break;
				}
			}

			// Pause handling
			while (paused.load()) {
				if (!running.load()) return;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				cpuCycles++;
				consoleManager.getMemoryManager().incrementIdleCpuTicks();
			}
			if (!running.load()) break;

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			cpuCycles++;
			consoleManager.getMemoryManager().incrementActiveCpuTicks();

			// Execute instruction
			cmd->execute(process, coreId);
			delete cmd;

			process->incrementCurrentLine();

			// Simulate delay-per-exec
			for (unsigned int i = 0; i < delayPerExec; ++i) {
				cpuCycles++;
				consoleManager.getMemoryManager().incrementActiveCpuTicks();
			}
		}

		if (!running.load()) break;

		// Only deallocate memory and mark process as completed if we finished all instructions
		if (process->getNextCommand() == nullptr) {
			process->setCompleted(true);
			process->log("Process finished execution.", coreId);
			consoleManager.getMemoryManager().deallocateMemory(process);
		}

		lock.lock();
		worker->busy.store(false);
		worker->currentProcess = nullptr;
		lock.unlock();
	}
}

int SchedulerFirstComeFirstServe::getTotalCores() const {
	return numCores;
}

int SchedulerFirstComeFirstServe::getBusyCores() const {
	int busyCores = 0;
	for (const Worker* worker : workers) {
		if (worker->busy.load() && worker->currentProcess != nullptr && worker->currentProcess->isInMemory()) {
			busyCores++;
		}
	}
	return busyCores;
}

std::map<Process*, int> SchedulerFirstComeFirstServe::getRunningProcesses() const {
	std::map<Process*, int> runningProcesses;
	for (const Worker* worker : workers) {
		if (worker->currentProcess != nullptr && worker->currentProcess->isInMemory()) {
			runningProcesses[worker->currentProcess] = worker->coreId;
		}
	}
	return runningProcesses;
}

std::vector<Process*> SchedulerFirstComeFirstServe::getQueuedProcesses() const {
	std::vector<Process*> queuedProcesses;
	{
		std::lock_guard<std::mutex> lock(queuedProcessesMutex);
		for (auto process : queuedProcessesSet) {
			queuedProcesses.push_back(process);
		}
	}
	return queuedProcesses;
}

std::vector<Process*> SchedulerFirstComeFirstServe::getFinishedProcesses() const {
	std::vector<Process*> finishedProcesses;
	std::lock_guard<std::mutex> lock(allProcessesMutex);
	auto runningProcesses = getRunningProcesses();
	for (Process* process : allProcesses) {
		if (process->isCompleted() && runningProcesses.find(process) == runningProcesses.end()) {
			finishedProcesses.push_back(process);
		}
	}
	return finishedProcesses;
}