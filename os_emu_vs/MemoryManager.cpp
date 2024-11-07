#include "MemoryManager.h"
#include <algorithm>
#include <filesystem>

MemoryManager* MemoryManager::sharedInstance = nullptr;

MemoryManager* MemoryManager::getInstance() {
    return sharedInstance;
}

void MemoryManager::initialize() {
    if (sharedInstance == nullptr) {
        sharedInstance = new MemoryManager();
    }
}

void MemoryManager::destroy() {
    delete sharedInstance;
    sharedInstance = nullptr;
}

void MemoryManager::initMemory(size_t totalMem, size_t frameSize, size_t procMemSize) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    totalMemory = totalMem;
    this->frameSize = frameSize;
    processMemorySize = procMemSize;
    currentQuantum = 0;

    // Initialize with one free block spanning all memory
    memoryBlocks.clear();
    memoryBlocks.emplace_back(0, totalMemory, true);
}

bool MemoryManager::allocateMemory(const std::string& processName) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    // First-fit allocation
    for (auto& block : memoryBlocks) {
        if (block.isFree && block.size >= processMemorySize) {
            // If block is exactly the right size
            if (block.size == processMemorySize) {
                block.isFree = false;
                block.processName = processName;
                return true;
            }

            // Split block
            size_t remainingSize = block.size - processMemorySize;
            block.size = processMemorySize;
            block.isFree = false;
            block.processName = processName;

            // Create new free block with remaining space
            memoryBlocks.emplace(
                std::next(std::find_if(memoryBlocks.begin(), memoryBlocks.end(),
                    [&](const MemoryBlock& b) { return b.startAddress == block.startAddress; })),
                block.startAddress + processMemorySize,
                remainingSize,
                true
            );

            return true;
        }
    }
    return false;
}

void MemoryManager::deallocateMemory(const std::string& processName) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    for (auto it = memoryBlocks.begin(); it != memoryBlocks.end(); ++it) {
        if (!it->isFree && it->processName == processName) {
            it->isFree = true;
            it->processName.clear();

            mergeAdjacentFreeBlocks();
            break;
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

void MemoryManager::mergeAdjacentFreeBlocks() {
    auto it = memoryBlocks.begin();
    while (it != memoryBlocks.end() && std::next(it) != memoryBlocks.end()) {
        if (it->isFree && std::next(it)->isFree) {
            it->size += std::next(it)->size;
            memoryBlocks.erase(std::next(it));
        }
        else {
            ++it;
        }
    }
}

std::string MemoryManager::generateTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_s(&tm_buf, &now_c);
    std::stringstream ss;
    ss << std::put_time(&tm_buf, "%m/%d/%Y %H:%M:%S");
    return ss.str();
}

void MemoryManager::generateMemorySnapshot(size_t quantumNumber) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    std::stringstream filename;
    filename << "memory_stamp_" << std::setw(2) << std::setfill('0') << quantumNumber << ".txt";

    std::ofstream outFile(filename.str());
    if (!outFile.is_open()) {
        throw std::runtime_error("Could not create memory snapshot file");
    }

    // Write header information
    outFile << "Timestamp: " << generateTimestamp() << "\n";
    outFile << "Number of processes in memory: " << getProcessCount() << "\n";
    outFile << "Total external fragmentation in KB: " << getExternalFragmentation() / 1024 << "\n\n";

    // Write memory map
    outFile << "----end---- = " << totalMemory << "\n";

    // Print each memory block
    for (const auto& block : memoryBlocks) {
        outFile << block.startAddress + block.size << "\n";
        if (!block.isFree) {
            outFile << block.processName << "\n";
        }
    }

    outFile << "----start----- = 0\n";
    outFile.close();
}

bool MemoryManager::isContiguousBlockAvailable(size_t size) const {
    for (const auto& block : memoryBlocks) {
        if (block.isFree && block.size >= size) {
            return true;
        }
    }
    return false;
}