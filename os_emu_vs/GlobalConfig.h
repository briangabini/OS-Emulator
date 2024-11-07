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

    size_t getMaxOverallMem() const;
    size_t getMemPerFrame() const;
    size_t getMemPerProc() const;

    bool isCalledOnce() const;
    void loadConfigFromFile(const std::string& filename);

private:
    GlobalConfig() = default;
    ~GlobalConfig() = default;
    GlobalConfig(GlobalConfig const&) {};                   // copy constructor is private
    GlobalConfig& operator=(GlobalConfig const&) {};        // assignment operator is private
    static GlobalConfig* sharedInstance;

    void parseLine(const std::string& line);

    int numCpu;
    SchedulingAlgorithm scheduler;
    int quantumCycles;
    int batchProcessFreq;
    int minIns;
    int maxIns;
    int delayPerExec;

    size_t maxOverallMem;
    size_t memPerFrame;
    size_t memPerProc;

    bool calledOnce = false;
};