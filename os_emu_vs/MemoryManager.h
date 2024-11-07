#pragma once
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <mutex>

class MemoryManager {
private:
    struct MemoryBlock {
        size_t startAddress;
        size_t size;
        std::string processName;
        bool isFree;

        MemoryBlock(size_t start, size_t s, bool free = true)
            : startAddress(start), size(s), isFree(free) {}
    };

    static MemoryManager* sharedInstance;
    std::mutex memoryMutex;

    // Memory configuration
    size_t totalMemory;
    size_t frameSize;
    size_t processMemorySize;
    size_t currentQuantum;
    std::vector<MemoryBlock> memoryBlocks;

    // Private constructor for singleton
    MemoryManager() = default;
    ~MemoryManager() = default;
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;

public:
    static MemoryManager* getInstance();
    static void initialize();
    static void destroy();

    // Initialization
    void initMemory(size_t totalMem, size_t frameSize, size_t procMemSize);

    // Memory allocation/deallocation
    bool allocateMemory(const std::string& processName);
    void deallocateMemory(const std::string& processName);

    // Memory status
    size_t getTotalMemory() const { return totalMemory; }
    size_t getFreeMemory() const;
    size_t getExternalFragmentation() const;
    size_t getProcessCount() const;

    // Memory snapshot generation
    void generateMemorySnapshot(size_t quantumNumber);
    void incrementQuantum() { currentQuantum++; }
    size_t getCurrentQuantum() const { return currentQuantum; }


private:
    void mergeAdjacentFreeBlocks();
    std::string generateTimestamp() const;
    void printMemoryMap(std::ofstream& outFile) const;
    bool isContiguousBlockAvailable(size_t size) const;
};