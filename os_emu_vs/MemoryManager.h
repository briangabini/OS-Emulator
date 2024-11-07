#pragma once
#include <string>
#include <list>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>

class MemoryManager {
private:
    struct MemoryBlock {
        size_t startAddress;
        size_t size;
        bool isFree;
        std::string processName;

        MemoryBlock(size_t start, size_t sz, bool free = true, std::string name = "")
            : startAddress(start), size(sz), isFree(free), processName(name) {}
    };

    static MemoryManager* sharedInstance;
    std::list<MemoryBlock> memoryBlocks;
    std::mutex memoryMutex;

    size_t totalMemory;
    size_t frameSize;
    size_t processMemorySize;
    size_t currentQuantum;

    // Private constructor for singleton
    MemoryManager() = default;

    void mergeAdjacentFreeBlocks();
    std::string generateTimestamp() const;

public:
    static MemoryManager* getInstance();
    static void initialize();
    static void destroy();

    void initMemory(size_t totalMem, size_t frameSize, size_t procMemSize);
    bool allocateMemory(const std::string& processName);
    void deallocateMemory(const std::string& processName);

    // Memory statistics and utilities
    size_t getFreeMemory() const;
    size_t getTotalMemory() const { return totalMemory; }
    size_t getExternalFragmentation() const;
    size_t getProcessCount() const;
    bool isContiguousBlockAvailable(size_t size) const;
    void generateMemorySnapshot(size_t quantumNumber);

    // Prevent copying and assignment
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;
};