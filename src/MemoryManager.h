#pragma once

#include <map>
#include <list>
#include <mutex>
#include <vector>
#include "Process.h"

class MemoryManager {
public:
    MemoryManager();
    ~MemoryManager();

    void initialize(unsigned int maxMem, unsigned int memPerFrame);

    bool allocateMemory(Process* process, unsigned int size);
    void deallocateMemory(Process* process);

    unsigned int getTotalMemory() const;
    unsigned int getUsedMemory() const;
    unsigned int getFreeMemory() const;
    double getMemoryUtilization() const;

    unsigned int getIdleCpuTicks() const;
    unsigned int getActiveCpuTicks() const;
    unsigned int getTotalCpuTicks() const;
    unsigned int getNumPagedIn() const;
    unsigned int getNumPagedOut() const;

    void incrementIdleCpuTicks();
    void incrementActiveCpuTicks();

    std::vector<std::pair<std::string, unsigned int>> getProcessesInMemory() const;

    bool isProcessInMemory(Process* process) const;
    bool isPaging() const;

private:
    mutable std::mutex memoryMutex;

    unsigned int maxMemory; // Total memory in KB
    unsigned int memPerFrame; // Memory per frame in KB
    unsigned int totalFrames;

    bool flatMemory; // True if flat memory allocator is used

    // For flat memory allocation
    unsigned int usedMemory;
    std::list<Process*> memoryQueue; // Processes in memory (order of arrival)
    std::map<Process*, unsigned int> processMemoryMap;

    // For paging allocation
    unsigned int numPagedIn;
    unsigned int numPagedOut;
    std::map<Process*, unsigned int> processPageMap; // Process to number of pages

    unsigned int idleCpuTicks;
    unsigned int activeCpuTicks;
    unsigned int totalCpuTicks;
};