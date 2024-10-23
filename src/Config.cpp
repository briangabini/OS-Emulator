#include "Config.h"
#include <fstream>
#include <sstream>
#include <iostream>

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

Config::Config()
    : numCpu(4), schedulerType("rr"), quantumCycles(5),
    batchProcessFreq(1), minIns(1000), maxIns(2000), delaysPerExec(0) {
}

bool Config::loadConfig(const std::string& filename) {
    std::ifstream configFile(filename);
    if (!configFile) {
        std::cerr << "Failed to open " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        // Ignore empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string paramName;
        iss >> paramName;

        if (paramName == "num-cpu") {
            iss >> numCpu;
            if (numCpu < 1 || numCpu > 128) {
                std::cerr << "Invalid num-cpu in " << filename << std::endl;
                return false;
            }
        }
        else if (paramName == "scheduler") {
            std::string schedulerValue;
            iss >> schedulerValue;
            schedulerType = stripQuotes(schedulerValue);
            if (schedulerType != "fcfs" && schedulerType != "rr") {
                std::cerr << "Invalid scheduler type in " << filename << std::endl;
                return false;
            }
        }
        else if (paramName == "quantum-cycles") {
            iss >> quantumCycles;
            if (quantumCycles < 1) {
                std::cerr << "Invalid quantum-cycles in " << filename << std::endl;
                return false;
            }
        }
        else if (paramName == "batch-process-freq") {
            iss >> batchProcessFreq;
            if (batchProcessFreq < 1) {
                std::cerr << "Invalid batch-process-freq in " << filename << std::endl;
                return false;
            }
        }
        else if (paramName == "min-ins") {
            iss >> minIns;
            if (minIns < 1) {
                std::cerr << "Invalid min-ins in " << filename << std::endl;
                return false;
            }
        }
        else if (paramName == "max-ins") {
            iss >> maxIns;
            if (maxIns < 1) {
                std::cerr << "Invalid max-ins in " << filename << std::endl;
                return false;
            }
        }
        else if (paramName == "delay-per-exec") {
            iss >> delaysPerExec;
            if (delaysPerExec < 0) {
                std::cerr << "Invalid delays-per-exec in " << filename << std::endl;
                return false;
            }
        }
        else {
            std::cerr << "Unknown parameter in " << filename << ": " << paramName << std::endl;
            return false;
        }
    }

    configFile.close();
    return true;
}

std::string Config::stripQuotes(const std::string& str) {
    if (str.size() >= 2 &&
        ((str.front() == '"' && str.back() == '"') ||
            (str.front() == '\'' && str.back() == '\''))) {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

int Config::getNumCpu() const {
    return numCpu;
}

const std::string& Config::getSchedulerType() const {
    return schedulerType;
}

unsigned int Config::getQuantumCycles() const {
    return quantumCycles;
}

unsigned int Config::getBatchProcessFreq() const {
    return batchProcessFreq;
}

unsigned int Config::getMinIns() const {
    return minIns;
}

unsigned int Config::getMaxIns() const {
    return maxIns;
}

unsigned int Config::getDelaysPerExec() const {
    return delaysPerExec;
}
