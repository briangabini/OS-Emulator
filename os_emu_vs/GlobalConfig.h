#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include "AScheduler.h"

class GlobalConfig
{
public:
	static GlobalConfig* getInstance();
	static void initialize();
	static void destroy();

    int getNumCpu() const;
    SchedulingAlgorithm getScheduler() const;
    int getQuantumCycles() const;
    int getBatchProcessFreq() const;
    int getMinIns() const;
    int getMaxIns() const;
    int getDelayPerExec() const;

    bool isCalledOnce() const;
    void loadConfigFromFile(const std::string& filename);

    // Memory Related
	int getMaxOverallMemory() const;
	int getMemoryPerFrame() const;
	int getMemoryPerProcess() const;
    bool isUsingFlatMemoryAllocator() const;
    int generateRandomPowerOf2(int minVal, int maxVal) const;
    int generateRandomNumberOfPages() const;

private:
    GlobalConfig() = default;
    ~GlobalConfig() = default;
	GlobalConfig(GlobalConfig const&) {};                   // copy constructor is private, disallow creation of a new instance
	GlobalConfig& operator=(GlobalConfig const&) {};        // assignment operator is private, disallow assignment
    static GlobalConfig* sharedInstance;

    void parseLine(const std::string& line);

    int numCpu;
    SchedulingAlgorithm scheduler;
    int quantumCycles;
    int batchProcessFreq;
    int minIns;
    int maxIns;
    int delayPerExec;

    // to ensure that it will only load once
    bool calledOnce = false;

    // week 8
    int maxOverallMemory;
    int memoryPerFrame;
    //int memoryPerProcess;

    // mco2
    int minMemPerProcess;
    int maxMemPerProcess;
	bool usingFlatMemoryAllocator;      // true -> flat memory allocator, false -> paging memory allocator
};
