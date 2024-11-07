#include "MemoryManager.h"
#include <algorithm>
#include <stdexcept>

MemoryManager* MemoryManager::sharedInstance = nullptr;

MemoryManager* MemoryManager::getInstance() {
    if (!sharedInstance) {
        throw std::runtime_error("MemoryManager not initialized");
    }
    return sharedInstance;
}

void MemoryManager::initialize() {
    if (!sharedInstance) {
        sharedInstance = new MemoryManager();
    }
}

void MemoryManager::destroy() {
    delete sharedInstance;
    sharedInstance = nullptr;
}

void MemoryManager::initMemory(size_t totalMem, size_t frameSz, size_t procMemSize) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    totalMemory = totalMem;
    frameSize = frameSz;
    processMemorySize = procMemSize;
    currentQuantum = 0;

    // Start with one large free block
    memoryBlocks.clear();
    memoryBlocks.emplace_back(0, totalMemory, true);
}

void MemoryManager::deallocateMemory(const std::string& processName) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    if (processName.empty()) {
        return;
    }

    for (auto it = memoryBlocks.begin(); it != memoryBlocks.end(); ++it) {
        if (!it->isFree && it->processName == processName) {
            it->isFree = true;
            it->processName.clear();
        }
    }

    mergeAdjacentFreeBlocks();
}

bool MemoryManager::allocateMemory(const std::string& processName) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    if (processName.empty()) {
        return false;
    }

    // Check if process already has memory allocated
    for (const auto& block : memoryBlocks) {
        if (!block.isFree && block.processName == processName) {
            return false;
        }
    }

    // Find suitable block
    for (auto it = memoryBlocks.begin(); it != memoryBlocks.end(); ++it) {
        if (it->isFree && it->size >= processMemorySize) {
            if (it->size == processMemorySize) {
                it->isFree = false;
                it->processName = processName;
                return true;
            }

            // Split block
            size_t remainingSize = it->size - processMemorySize;
            it->size = processMemorySize;
            it->isFree = false;
            it->processName = processName;

            memoryBlocks.emplace(std::next(it),
                it->startAddress + processMemorySize,
                remainingSize,
                true);

            return true;
        }
    }

    return false;
}

void MemoryManager::mergeAdjacentFreeBlocks() {
    if (memoryBlocks.empty()) return;

    auto current = memoryBlocks.begin();
    while (current != memoryBlocks.end() && std::next(current) != memoryBlocks.end()) {
        auto next = std::next(current);
        if (current->isFree && next->isFree) {
            current->size += next->size;
            memoryBlocks.erase(next);
        }
        else {
            ++current;
        }
    }
}

size_t MemoryManager::getFreeMemory() const {
    size_t freeMemory = 0;
    for (const auto& block : memoryBlocks) {
        if (block.isFree) {
            freeMemory += block.size;
        }
    }
    return freeMemory;
}

size_t MemoryManager::getExternalFragmentation() const {
    size_t fragmentation = 0;
    for (const auto& block : memoryBlocks) {
        if (block.isFree && block.size < processMemorySize) {
            fragmentation += block.size;
        }
    }
    return fragmentation;
}

size_t MemoryManager::getProcessCount() const {
    return std::count_if(memoryBlocks.begin(), memoryBlocks.end(),
        [](const MemoryBlock& block) { return !block.isFree; });
}

bool MemoryManager::isContiguousBlockAvailable(size_t size) const {
    return std::any_of(memoryBlocks.begin(), memoryBlocks.end(),
        [size](const MemoryBlock& block) { return block.isFree && block.size >= size; });
}

std::string MemoryManager::generateTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm timeinfo;
#ifdef _WIN32
    localtime_s(&timeinfo, &time);
#else
    localtime_r(&time, &timeinfo);
#endif
    std::stringstream ss;
    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void MemoryManager::generateMemorySnapshot(size_t quantumNumber) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    std::stringstream filename;
    filename << "memory_snapshot_" << std::setw(4) << std::setfill('0') << quantumNumber << ".txt";

    std::ofstream outFile(filename.str());
    if (!outFile) {
        throw std::runtime_error("Failed to create memory snapshot file");
    }

    outFile << "Memory Snapshot - Quantum " << quantumNumber << "\n";
    outFile << "Timestamp: " << generateTimestamp() << "\n";
    outFile << "Total Memory: " << totalMemory << " bytes\n";
    outFile << "Free Memory: " << getFreeMemory() << " bytes\n";
    outFile << "Process Count: " << getProcessCount() << "\n";
    outFile << "External Fragmentation: " << getExternalFragmentation() << " bytes\n\n";

    outFile << "Memory Map:\n";
    outFile << "------------\n";
    for (const auto& block : memoryBlocks) {
        outFile << "Address: " << std::setw(10) << block.startAddress
            << " Size: " << std::setw(10) << block.size
            << " Status: " << (block.isFree ? "Free" : "Used");
        if (!block.isFree) {
            outFile << " Process: " << block.processName;
        }
        outFile << "\n";
    }

    outFile.close();
}