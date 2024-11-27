#include "MemoryManager.h"
#include <algorithm>

MemoryManager::MemoryManager()
    : maxMemory(0), memPerFrame(0), totalFrames(0), flatMemory(true),
    usedMemory(0), numPagedIn(0), numPagedOut(0),
    idleCpuTicks(0), activeCpuTicks(0), totalCpuTicks(0) {}

MemoryManager::~MemoryManager() {}

void MemoryManager::initialize(unsigned int maxMem, unsigned int memPerFrame) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    maxMemory = maxMem;
    this->memPerFrame = memPerFrame;
    totalFrames = maxMemory / memPerFrame;
    flatMemory = (maxMemory == memPerFrame);

    if (flatMemory) {
        // Initialize single block of free memory
        memoryBlocks.clear();
        memoryBlocks.push_back({ 0, maxMemory, nullptr });
    }
    else {
        // Initialize frames for paging
        frames.resize(totalFrames);
        for (auto& frame : frames) {
            frame.allocated = false;
            frame.owner = nullptr;
            frame.pageNumber = -1;
        }
    }
}

void MemoryManager::compactMemory() {
    if (!flatMemory) return;

    std::vector<MemoryBlock> newBlocks;
    size_t currentOffset = 0;

    // Move all allocated blocks to the front
    for (const auto& block : memoryBlocks) {
        if (block.process != nullptr) {
            newBlocks.push_back({ currentOffset, block.size, block.process });
            currentOffset += block.size;
        }
    }

    // Add remaining space as a single free block
    if (currentOffset < maxMemory) {
        newBlocks.push_back({ currentOffset, maxMemory - currentOffset, nullptr });
    }

    memoryBlocks = std::move(newBlocks);
}

bool MemoryManager::findFreeFrames(unsigned int numFramesNeeded, std::vector<int>& frameNumbers) {
    frameNumbers.clear();
    for (size_t i = 0; i < frames.size() && frameNumbers.size() < numFramesNeeded; ++i) {
        if (!frames[i].allocated) {
            frameNumbers.push_back(i);
        }
    }
    return frameNumbers.size() == numFramesNeeded;
}

void MemoryManager::removeOldestProcess() {
    if (memoryQueue.empty()) return;

    Process* oldestProcess = memoryQueue.front();
    memoryQueue.pop_front();

    if (flatMemory) {
        for (auto& block : memoryBlocks) {
            if (block.process == oldestProcess) {
                block.process = nullptr;
                usedMemory -= block.size;
            }
        }
        compactMemory();
    }
    else {
        auto it = pageTables.find(oldestProcess);
        if (it != pageTables.end()) {
            for (const auto& entry : it->second) {
                if (entry.present) {
                    frames[entry.frameNumber].allocated = false;
                    frames[entry.frameNumber].owner = nullptr;
                    frames[entry.frameNumber].pageNumber = -1;
                    numPagedOut++;
                }
            }
            pageTables.erase(it);
        }
    }

    oldestProcess->setInMemory(false);
}

bool MemoryManager::allocateMemory(Process* process, unsigned int size) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    if (size > maxMemory) {
        return false;
    }

    if (flatMemory) {
        // First, try to find a suitable free block
        for (auto& block : memoryBlocks) {
            if (block.process == nullptr && block.size >= size) {
                // Split block if it's larger than needed
                if (block.size > size) {
                    memoryBlocks.push_back({
                        block.offset + size,
                        block.size - size,
                        nullptr
                        });
                }
                block.size = size;
                block.process = process;
                usedMemory += size;
                memoryQueue.push_back(process);
                process->setInMemory(true);
                return true;
            }
        }

        // If no suitable block found, try to free up space
        while (getFreeMemory() < size && !memoryQueue.empty()) {
            removeOldestProcess();
            compactMemory();
        }

        // Try allocation again
        if (getFreeMemory() >= size) {
            for (auto& block : memoryBlocks) {
                if (block.process == nullptr && block.size >= size) {
                    if (block.size > size) {
                        memoryBlocks.push_back({
                            block.offset + size,
                            block.size - size,
                            nullptr
                            });
                    }
                    block.size = size;
                    block.process = process;
                    usedMemory += size;
                    memoryQueue.push_back(process);
                    process->setInMemory(true);
                    return true;
                }
            }
        }
    }
    else {
        // Paging allocation
        unsigned int numPages = (size + memPerFrame - 1) / memPerFrame;
        std::vector<int> freeFrames;

        while (!findFreeFrames(numPages, freeFrames) && !memoryQueue.empty()) {
            removeOldestProcess();
        }

        if (findFreeFrames(numPages, freeFrames)) {
            std::vector<PageTableEntry> pageTable(numPages);
            for (size_t i = 0; i < numPages; ++i) {
                int frameNum = freeFrames[i];
                frames[frameNum].allocated = true;
                frames[frameNum].owner = process;
                frames[frameNum].pageNumber = i;

                pageTable[i].frameNumber = frameNum;
                pageTable[i].present = true;
                numPagedIn++;
            }

            pageTables[process] = std::move(pageTable);
            memoryQueue.push_back(process);
            process->setInMemory(true);
            return true;
        }
    }

    return false;
}

void MemoryManager::deallocateMemory(Process* process) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    if (flatMemory) {
        for (auto& block : memoryBlocks) {
            if (block.process == process) {
                usedMemory -= block.size;
                block.process = nullptr;
            }
        }
        compactMemory();
    }
    else {
        auto it = pageTables.find(process);
        if (it != pageTables.end()) {
            for (const auto& entry : it->second) {
                if (entry.present) {
                    frames[entry.frameNumber].allocated = false;
                    frames[entry.frameNumber].owner = nullptr;
                    frames[entry.frameNumber].pageNumber = -1;
                }
            }
            pageTables.erase(it);
        }
    }

    memoryQueue.remove(process);
    process->setInMemory(false);
}

unsigned int MemoryManager::getUsedMemory() const {
    std::lock_guard<std::mutex> lock(memoryMutex);

    if (flatMemory) {
        return usedMemory;
    }
    else {
        unsigned int usedFrames = 0;
        for (const auto& frame : frames) {
            if (frame.allocated) {
                usedFrames++;
            }
        }
        return usedFrames * memPerFrame;
    }
}

unsigned int MemoryManager::getTotalMemory() const {
    return maxMemory;
}

unsigned int MemoryManager::getFreeMemory() const {
    return getTotalMemory() - getUsedMemory();
}

double MemoryManager::getMemoryUtilization() const {
    return (static_cast<double>(getUsedMemory()) / getTotalMemory()) * 100.0;
}

std::vector<std::pair<Process*, unsigned int>> MemoryManager::getProcessesInMemory() const {
    std::lock_guard<std::mutex> lock(memoryMutex);
    std::vector<std::pair<Process*, unsigned int>> result;

    if (flatMemory) {
        std::map<Process*, unsigned int> processMemory;
        for (const auto& block : memoryBlocks) {
            if (block.process != nullptr) {
                processMemory[block.process] += block.size;
            }
        }
        for (const auto& pair : processMemory) {
            result.emplace_back(pair.first, pair.second);
        }
    }
    else {
        for (const auto& pair : pageTables) {
            unsigned int numPages = 0;
            for (const auto& entry : pair.second) {
                if (entry.present) numPages++;
            }
            result.emplace_back(pair.first, numPages * memPerFrame);
        }
    }

    return result;
}

bool MemoryManager::isProcessInMemory(Process* process) const {
    std::lock_guard<std::mutex> lock(memoryMutex);
    return std::find(memoryQueue.begin(), memoryQueue.end(), process) != memoryQueue.end();
}

bool MemoryManager::isPaging() const {
    return !flatMemory;
}

unsigned int MemoryManager::getIdleCpuTicks() const {
    return idleCpuTicks;
}

unsigned int MemoryManager::getActiveCpuTicks() const {
    return activeCpuTicks;
}

unsigned int MemoryManager::getTotalCpuTicks() const {
    return totalCpuTicks;
}

unsigned int MemoryManager::getNumPagedIn() const {
    return numPagedIn;
}

unsigned int MemoryManager::getNumPagedOut() const {
    return numPagedOut;
}

void MemoryManager::incrementIdleCpuTicks() {
    std::lock_guard<std::mutex> lock(memoryMutex);
    idleCpuTicks++;
    totalCpuTicks++;
}

void MemoryManager::incrementActiveCpuTicks() {
    std::lock_guard<std::mutex> lock(memoryMutex);
    activeCpuTicks++;
    totalCpuTicks++;
}