#include "GlobalConfig.h"
#include <random>
#include <cmath>
#include <iostream>

GlobalConfig* GlobalConfig::sharedInstance = nullptr;

GlobalConfig* GlobalConfig::getInstance()
{
	return sharedInstance;
}

void GlobalConfig::initialize() {
	if (sharedInstance == nullptr) {
		sharedInstance = new GlobalConfig();
	}
}

void GlobalConfig::destroy()
{
	delete sharedInstance;
	sharedInstance = nullptr;
}

int GlobalConfig::getNumCpu() const {
	return numCpu;
}

SchedulingAlgorithm GlobalConfig::getScheduler() const {
	return scheduler;
}

int GlobalConfig::getQuantumCycles() const {
	return quantumCycles;
}

int GlobalConfig::getBatchProcessFreq() const {
	return batchProcessFreq;
}

int GlobalConfig::getMinIns() const {
	return minIns;
}

int GlobalConfig::getMaxIns() const {
	return maxIns;
}

int GlobalConfig::getDelayPerExec() const {
	return delayPerExec;
}

bool GlobalConfig::isCalledOnce() const {
	return calledOnce;
}

int GlobalConfig::getMaxOverallMemory() const {
	return maxOverallMemory;
}

int GlobalConfig::getMemoryPerFrame() const {
	return memoryPerFrame;
}

// Add this private helper function to the GlobalConfig class
int GlobalConfig::generateRandomPowerOf2(int minVal, int maxVal) const {
	// Define the range of powers of 2
	const auto minPower = static_cast<int>(std::log2(minVal));
	const auto maxPower = static_cast<int>(std::log2(maxVal));

	// Seed for the random number engine
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution dis(minPower, maxPower);

	// Generate a random power of 2 within the specified range
	const int power = dis(gen);
	return static_cast<int>(std::pow(2, power));
}

int GlobalConfig::getMemoryPerProcess() const {
	// return generateRandomPowerOf2(minMemPerProcess, maxMemPerProcess);
	return minMemPerProcess;
}

int GlobalConfig::generateRandomNumberOfPages() const {
	const int M = generateRandomPowerOf2(minMemPerProcess, maxMemPerProcess);

	/*std::cout << "power: " << power << std::endl;
	std::cout << "M: " << M << std::endl;
	std::cout << "memoryPerFrame: " << memoryPerFrame << std::endl;
	std::cout << "M / memoryPerFrame: " << M / memoryPerFrame << std::endl;
	std::cout << "-----------------" << std::endl;*/

	// HOTFIX: if M is less than memoryPerFrame, return 1
	if (M < memoryPerFrame) {
		return 1;
	}
	else {
		return M / memoryPerFrame;
	}
}

bool GlobalConfig::isUsingFlatMemoryAllocator() const {
	return usingFlatMemoryAllocator;
}

void GlobalConfig::loadConfigFromFile(const std::string& filename)
{
	if (calledOnce)
	{
		return;
	}

	std::ifstream file(filename);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open config file.");
	}

	std::string line;
	while (std::getline(file, line))
	{
		parseLine(line);
	}

	calledOnce = true;

	/* FOR DEBUGGING */
	/*std::cout << "max-overall-mem: " << maxOverallMemory << std::endl;
	std::cout << "mem-per-frame: " << memoryPerFrame << std::endl;
	std::cout << "min-memory-per-process: " << minMemPerProcess << std::endl;
	std::cout << "max-memory-per-process: " << maxMemPerProcess << std::endl;
	std::cout << "usingFlatMemoryAllocator: " << usingFlatMemoryAllocator << std::endl;

	for (size_t i = 0; i < 5; ++i) {
		int numPages = generateRandomNumberOfPages();
		std::cout << "Random number of pages: " << numPages << std::endl;
	}*/
}


void GlobalConfig::parseLine(const std::string& line)
{
	std::istringstream iss(line);
	std::string key;

	if (std::getline(iss, key, ' '))
	{
		std::string value;

		if (std::getline(iss, value))
		{
			if (key == "num-cpu") {
				numCpu = std::stoi(value);
			}
			else if (key == "scheduler") {
				if (value == "\"rr\"")
				{
					scheduler = SchedulingAlgorithm::ROUND_ROBIN;
				}
				else if (value == "\"fcfs\"")
				{
					scheduler = SchedulingAlgorithm::FCFS;
				}
			}
			else if (key == "quantum-cycles") {
				quantumCycles = std::stoi(value);
			}
			else if (key == "batch-process-freq") {
				batchProcessFreq = std::stoi(value);
			}
			else if (key == "min-ins") {
				minIns = std::stoi(value);
			}
			else if (key == "max-ins") {
				maxIns = std::stoi(value);
			}
			else if (key == "max-overall-mem") {
				maxOverallMemory = std::stoi(value);
			}
			else if (key == "mem-per-frame") {
				memoryPerFrame = std::stoi(value);
			}
			else if (key == "min-mem-per-proc") {
				minMemPerProcess = std::stoi(value);
			}
			else if (key == "max-mem-per-proc") {
				maxMemPerProcess = std::stoi(value);
			}
			else if (key == "delay-per-exec") {
				delayPerExec = std::stoi(value);
			}
			else {
				// Handle unknown key
				std::cerr << "Unknown configuration key: " << key << std::endl;
			}
		}
	}

	// check if using flat memory allocator
	// condition: max-overall-mem == mem-per-frame
	if (maxOverallMemory == memoryPerFrame)
	{
		usingFlatMemoryAllocator = true;
	}
	else
	{
		usingFlatMemoryAllocator = false;
	}
}