#include "FCFSScheduler.h"
#include "GlobalScheduler.h"
#include <format>
#include <iostream>
#include <thread>

GlobalScheduler* GlobalScheduler::sharedInstance = nullptr;
GlobalScheduler::GlobalScheduler() {
	scheduler = std::make_shared<FCFSScheduler>();
	this->start();
	scheduler->start();
	// test_init10Processes();
}

void GlobalScheduler::initialize()
{
	if (sharedInstance == nullptr) {
		sharedInstance = new GlobalScheduler();
	}
}

GlobalScheduler* GlobalScheduler::getInstance() {
	return sharedInstance;
}

void GlobalScheduler::destroy() {
	delete sharedInstance;
	sharedInstance = nullptr;
}

void GlobalScheduler::tick() const {
	scheduler->execute();
}

std::shared_ptr<Process> GlobalScheduler::createUniqueProcess(std::string processName) {
	if (processes.find(processName) != processes.end()) {
		return nullptr;
	}

	static int nextPid = 1;
	std::shared_ptr<Process> newProcess = std::make_shared<Process>(nextPid++, processName);

	// Week 6
	for (int i = 0; i < 100; i++) {
		newProcess->addCommand(ICommand::PRINT);
	}

	processes[processName] = newProcess;

	if (scheduler) {
		scheduler->addProcess(newProcess);
	}

	return newProcess;
}

void GlobalScheduler::create10DummyProcesses() {
	for (int i = 0; i < 10; i++) {
		GlobalScheduler::getInstance()->createUniqueProcess("process_" + std::to_string(i));
	}
}


std::shared_ptr<Process> GlobalScheduler::findProcess(String& name) const {
	auto it = processes.find(name);
	if (it != processes.end()) {
		return it->second;
	}
	return nullptr;
}

void GlobalScheduler::test_init10Processes() {
	for (int i = 1; i <= 10; ++i) {
		String processName = std::format("process_{}", i);
		auto process = createUniqueProcess(processName);
		process->test_generate100PrintCommands();
	}
}

bool GlobalScheduler::allProcessesFinished() const {
	for (const auto& [name, process] : processes) {
		if (process->getState() != Process::FINISHED) {
			return false;
		}
	}
	return true;
}

void GlobalScheduler::run() {
	this->scheduler->execute();
}

void GlobalScheduler::listProcesses() const {
	if (processes.empty()) {
		std::cout << "No processes found. \n";
		return;
	}

	for (const auto& p : processes) {
		auto process = p.second;

		std::string status;
		if (process->isFinished()) {
			status = "Finished";
		}

		else {
			status = "Core: " + std::to_string(process->getCPUCoreID());
		}

		int currentLinesOfCode = process->getCommandCounter();
		int totalLinesOfCode = process->getLinesOfCode();

		std::string coreIdOutput;
		if (process->isFinished()) {
			coreIdOutput = "Finished";
		}
		else if (process->getCPUCoreID() == -1) {
			coreIdOutput = "N/A";
		}
		else {
			coreIdOutput = std::to_string(process->getCPUCoreID());
		}

		std::cout << "Process Name: " << process->getName()
			<< " | PID: " << process->getPID()
			<< " | Status: " << (process->isFinished() ? "Finished" : "Not Finished")
			<< " | Core ID: " << coreIdOutput
			<< " | Lines of Code: " << currentLinesOfCode << "/" << totalLinesOfCode
			<< "\n";
	}
}

void GlobalScheduler::monitorProcesses() const {
	std::cout << "---------------------------------------\n";
	std::cout << "Running processes:\n";
	for (const auto& [name, process] : processes) {
		if (process->getState() != Process::FINISHED) {
			std::cout << name << "\t" << formatTimestamp(process->getCreationTime()) << "\tCore: " << process->getCPUCoreID() << (process->getCPUCoreID() == -1 ? " " : "\t") << process->getCommandCounter() << " / " << process->getLinesOfCode() << "\n";
		}
	}
	std::cout << "\nFinished processes:\n";
	for (const auto& [name, process] : processes) {
		if (process->getState() == Process::FINISHED) {
			std::cout << name << "\t" << formatTimestamp(process->getCreationTime()) << "\tFinished\t" << process->getLinesOfCode() << " / " << process->getLinesOfCode() << "\n";
		}
	}
	std::cout << "---------------------------------------\n";
}

std::string formatTimestamp(const std::chrono::time_point<std::chrono::system_clock>& timePoint) {
	std::time_t timeT = std::chrono::system_clock::to_time_t(timePoint);
	std::tm tm;
	localtime_s(&tm, &timeT);
	std::ostringstream oss;
	oss << std::put_time(&tm, "(%m/%d/%Y %I:%M:%S%p)");
	return oss.str();
}