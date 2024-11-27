#include "FCFSScheduler.h"
#include "GlobalScheduler.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <thread>

#include "Util.h"
#include "BaseScreen.h"
#include "ConsoleManager.h"
#include "MemoryManager.h"
#include "GlobalConfig.h"
#include "os_emu_vs.h"
#include "RRScheduler.h"


GlobalScheduler* GlobalScheduler::sharedInstance = nullptr;
GlobalScheduler::GlobalScheduler(SchedulingAlgorithm algo) {

	if (algo == SchedulingAlgorithm::FCFS) {
		scheduler = std::make_shared<FCFSScheduler>();
	}
	else if (algo == SchedulingAlgorithm::ROUND_ROBIN) {
		scheduler = std::make_shared<RRScheduler>();
	}

	this->start();
	scheduler->start();
}

void GlobalScheduler::initialize()
{
	SchedulingAlgorithm algo = GlobalConfig::getInstance()->getScheduler();

	if (sharedInstance == nullptr) {
		sharedInstance = new GlobalScheduler(algo);
	}
}

GlobalScheduler* GlobalScheduler::getInstance() {
	return sharedInstance;
}

void GlobalScheduler::destroy() {
	delete sharedInstance;
	sharedInstance = nullptr;
}

GlobalScheduler::~GlobalScheduler()
{
	stopSchedulerTest();
}

void GlobalScheduler::startSchedulerTest()
{
	schedulerTestRunning = true;
	processGeneratorThread = std::thread(&GlobalScheduler::generateProcesses, this);
}

void GlobalScheduler::stopSchedulerTest()
{
	schedulerTestRunning = false;
	if (processGeneratorThread.joinable())
	{
		processGeneratorThread.join();
	}
}

void GlobalScheduler::generateProcesses()
{
	bool processCreatedInCurrentCycle = false;

	int batchProcessFrequency = GlobalConfig::getInstance()->getBatchProcessFreq();

	while (schedulerTestRunning)
	{

		if (cpuCycles % (batchProcessFrequency + 1) == 0 && !processCreatedInCurrentCycle)
		{
			const auto newProcess = createProcess("process_", Mode::KERNEL);
			const auto newBaseScreen = std::make_shared<BaseScreen>(newProcess, newProcess->getName());

			processCreatedInCurrentCycle = true;

			try {
				ConsoleManager::getInstance()->registerScreen(newBaseScreen);
			}
			catch (const std::exception& e) {
				return;
			}
		}
		else if (cpuCycles % (batchProcessFrequency + 1) != 0)
		{
			processCreatedInCurrentCycle = false;
		}
	}
}


void GlobalScheduler::tick() const {
	scheduler->execute();
}

std::shared_ptr<Process> GlobalScheduler::createProcess(std::string processName, Mode mode) {
	if (processes.find(processName) != processes.end()) {
		return nullptr;
	}

	static int nextPid = 1;
	String newProcessName;
	if (mode == Mode::KERNEL)
	{
		newProcessName = processName + std::to_string(nextPid);
	}
	else if (mode == Mode::USER) {
		newProcessName = processName;
	}

	std::shared_ptr<Process> newProcess = std::make_shared<Process>(nextPid++, newProcessName);

	// Set the memory required by assigning the memory per process to the process
	newProcess->setMemoryRequired(GlobalConfig::getInstance()->getMemoryPerProcess());

	// processSize contains the memory required by the process initialized above^
	size_t processSize = newProcess->getMemoryRequired();

	newProcess->addCommand(ICommand::PRINT);

	processes[newProcessName] = newProcess;

	if (scheduler) {
		scheduler->addProcess(newProcess);
	}

	return newProcess;
}

std::shared_ptr<Process> GlobalScheduler::findProcess(String& name) const {
	auto it = processes.find(name);
	return (it != processes.end()) ? it->second : nullptr;
}

// from video
void GlobalScheduler::run() {
	this->scheduler->execute();
}

void GlobalScheduler::monitorProcesses() const {
	std::ostringstream oss;

		oss.str(""); // Clear the string stream
		oss << "CPU utilization: " << scheduler->getCpuUtilization() << "%\n";
		oss << "Cores used: " << scheduler->getActiveWorkersCount() << "\n";
		oss << "Cores available: " << scheduler->workersCount - scheduler->getActiveWorkersCount() << "\n\n";

		oss << "---------------------------------------\n";
		oss << "Running processes:\n";


		for (const auto& [name, process] : processes) {
			if (process->getState() != Process::FINISHED) {
				if (process->getCreationTime() == std::chrono::system_clock::time_point()) {
					oss << name << "\t(Invalid Time)\t\tCore: (Invalid)\t(Invalid) / (Invalid)\t\tNumber of Pages: (Invalid)\n";
				}
				else {
					oss << name << "\t" << formatTimestamp(process->getCreationTime()) << "\t\tCore: " << process->getCPUCoreID() << (process->getCPUCoreID() == -1 ? " " : " \t") << process->getCommandCounter() << " / " << process->getLinesOfCode() << "\t\tNumber of Pages: " << process->getNumberOfPages() << "\n";
				}
			}
		}

		oss << "\nFinished processes:\n";
		for (const auto& [name, process] : processes) {
			if (process->getState() == Process::FINISHED) {
				if (process->getCreationTime() == std::chrono::system_clock::time_point()) {
					oss << name << "\t(Invalid Time)\t\tFinished\t(Invalid) / (Invalid)\n";
				}
				else {
					oss << name << "\t" << formatTimestamp(process->getCreationTime()) << "\t\tFinished\t" << process->getLinesOfCode() << " / " << process->getLinesOfCode() << "\n";
				}
			}
		}
		oss << "---------------------------------------\n";


	lastMonitorOutput = oss.str();
	std::cout << lastMonitorOutput;
}


void GlobalScheduler::logToFile() const {
	std::ofstream logFile("report.txt", std::ofstream::trunc);
	if (logFile.is_open()) {
		logFile << lastMonitorOutput;
	}
	logFile.close();
	std::filesystem::path filePath = std::filesystem::absolute("report.txt");
	std::cout << "Report generated at " << filePath.string() << "!\n";
}

void GlobalScheduler::logMemory() const {
	int processCount = MemoryManager::getInstance()->getProcessCount();

	// Open file for logging memory snapshot
	static int quantumCycle = 0;  // Track quantum cycle number
	std::ofstream outFile(std::format("logs/memory_stamp_{}.txt", quantumCycle++));

	if (!outFile.is_open()) {
		std::cerr << "Error opening file!\n";
		return;
	}

	// Write timestamp
	outFile << std::format("Timestamp: {}\n", Util::getCurrentTimestamp());

	// Write number of processes in memory
	outFile << std::format("Number of processes in memory: {}\n", processCount);

	// Write ASCII printout of memory (assuming visualizeMemory returns a formatted string)``
	outFile << MemoryManager::getInstance()->getMemoryAllocator()->visualizeMemory();

	outFile.close();
}

std::string formatTimestamp(const std::chrono::time_point<std::chrono::system_clock>& timePoint) {
	std::time_t timeT = std::chrono::system_clock::to_time_t(timePoint);
	std::tm tm = {};
	if (localtime_s(&tm, &timeT) != 0) {
		std::cerr << "Error converting time to local time." << std::endl;
		return "(Invalid Time)";
	}
	std::ostringstream oss;
	oss << std::put_time(&tm, "(%m/%d/%Y %I:%M:%S%p)");
	return oss.str();
}