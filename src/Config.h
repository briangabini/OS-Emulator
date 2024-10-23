#pragma once

#include <string>

class Config {
public:
    static Config& getInstance();

    bool loadConfig(const std::string& filename);

    int getNumCpu() const;
    const std::string& getSchedulerType() const;
    unsigned int getQuantumCycles() const;
    unsigned int getBatchProcessFreq() const;
    unsigned int getMinIns() const;
    unsigned int getMaxIns() const;
    unsigned int getDelaysPerExec() const;

private:
    Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::string stripQuotes(const std::string& str);

    int numCpu;
    std::string schedulerType;
    unsigned int quantumCycles;
    unsigned int batchProcessFreq;
    unsigned int minIns;
    unsigned int maxIns;
    unsigned int delaysPerExec;
};
