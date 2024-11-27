#pragma once

#include <map>
#include <list>
#include <mutex>
#include <vector>
#include "Process.h"

struct MemoryBlock {
    size_t offset;
    size_t size;
    Process* process;
};

struct Frame {
    bool allocated;
    Process* owner;
    int pageNumber;
};

struct PageTableEntry {
    int frameNumber;
    bool present;
};

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

    std::vector<std::pair<Process*, unsigned int>> getProcessesInMemory() const;
    bool isProcessInMemory(Process* process) const;
    bool isPaging() const;

private:
    void compactMemory();
    bool findFreeFrames(unsigned int numFramesNeeded, std::vector<int>& frameNumbers);
    void removeOldestProcess();

    mutable std::mutex memoryMutex;
    unsigned int maxMemory;
    unsigned int memPerFrame;
    unsigned int totalFrames;
    bool flatMemory;

    // For flat memory allocation
    std::vector<MemoryBlock> memoryBlocks;
    unsigned int usedMemory;

    // For paging allocation
    std::vector<Frame> frames;
    std::map<Process*, std::vector<PageTableEntry>> pageTables;

    std::list<Process*> memoryQueue;
    unsigned int numPagedIn;
    unsigned int numPagedOut;
    unsigned int idleCpuTicks;
    unsigned int activeCpuTicks;
    unsigned int totalCpuTicks;
};