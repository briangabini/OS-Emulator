#include "GlobalConfig.h"
#include "MemoryManager.h" 

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

    // First initialize the singleton, then init the memory
    MemoryManager::initialize();
    MemoryManager::getInstance()->initMemory(maxOverallMem, memPerFrame, memPerProc);

    calledOnce = true;
}

size_t GlobalConfig::getMaxOverallMem() const {
	return maxOverallMem;
}

size_t GlobalConfig::getMemPerFrame() const {
	return memPerFrame;
}

size_t GlobalConfig::getMemPerProc() const {
	return memPerProc;
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
            else if (key == "delay-per-exec") {
                delayPerExec = std::stoi(value);
            }
            // Add new memory management configuration parsing
            else if (key == "max-overall-mem") {
                maxOverallMem = std::stoul(value);
            }
            else if (key == "mem-per-frame") {
                memPerFrame = std::stoul(value);
            }
            else if (key == "mem-per-proc") {
                memPerProc = std::stoul(value);
            }
            else {
                // Handle unknown key
                std::cerr << "Unknown configuration key: " << key << std::endl;
            }
        }
    }
}