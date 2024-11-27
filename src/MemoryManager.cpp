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

/*
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
*/

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
                break; // Assuming one block per process
            }
        }
        // Merge adjacent free blocks
        mergeAdjacentFreeBlocks();

        // Mark process as swapped out
        swappedOutProcesses.insert(oldestProcess);
        oldestProcess->setInMemory(false);
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
        // Mark process as swapped out
        swappedOutProcesses.insert(oldestProcess);
        oldestProcess->setInMemory(false);
    }
}

void MemoryManager::mergeAdjacentFreeBlocks() {
    for (size_t i = 0; i < memoryBlocks.size() - 1; ) {
        if (memoryBlocks[i].process == nullptr && memoryBlocks[i + 1].process == nullptr) {
            // Merge blocks
            memoryBlocks[i].size += memoryBlocks[i + 1].size;
            memoryBlocks.erase(memoryBlocks.begin() + i + 1);
        }
        else {
            ++i;
        }
    }
}

bool MemoryManager::allocateMemory(Process* process, unsigned int size) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    if (size > maxMemory) {
        return false;
    }

    if (flatMemory) {
        // Try to find a suitable free block or merge adjacent free blocks
        while (true) {
            for (size_t i = 0; i < memoryBlocks.size(); ++i) {
                if (memoryBlocks[i].process == nullptr) {
                    size_t combinedSize = memoryBlocks[i].size;
                    size_t startIndex = i;
                    size_t endIndex = i;

                    // Check if current block is large enough
                    if (combinedSize >= size) {
                        // Allocate within this block
                        if (combinedSize > size) {
                            // Split the block
                            MemoryBlock newBlock = {
                                memoryBlocks[i].offset + size,
                                memoryBlocks[i].size - size,
                                nullptr
                            };
                            memoryBlocks.insert(memoryBlocks.begin() + i + 1, newBlock);
                        }

                        // Assign process to block
                        memoryBlocks[i].size = size;
                        memoryBlocks[i].process = process;
                        usedMemory += size;
                        memoryQueue.push_back(process);
                        process->setInMemory(true);
                        return true;
                    }

                    // Try to merge adjacent free blocks
                    for (size_t j = i + 1; j < memoryBlocks.size(); ++j) {
                        if (memoryBlocks[j].process == nullptr &&
                            memoryBlocks[j].offset == memoryBlocks[endIndex].offset + memoryBlocks[endIndex].size) {
                            combinedSize += memoryBlocks[j].size;
                            endIndex = j;

                            if (combinedSize >= size) {
                                // Merge blocks from startIndex to endIndex
                                memoryBlocks[startIndex].size = combinedSize;
                                memoryBlocks[startIndex].process = process;
                                usedMemory += size;
                                memoryQueue.push_back(process);
                                process->setInMemory(true);

                                // Remove merged blocks except the first one
                                memoryBlocks.erase(memoryBlocks.begin() + startIndex + 1, memoryBlocks.begin() + endIndex + 1);

                                // If there's extra space, split the block
                                if (combinedSize > size) {
                                    MemoryBlock newBlock = {
                                        memoryBlocks[startIndex].offset + size,
                                        combinedSize - size,
                                        nullptr
                                    };
                                    memoryBlocks.insert(memoryBlocks.begin() + startIndex + 1, newBlock);
                                    memoryBlocks[startIndex].size = size;
                                }

                                return true;
                            }
                        }
                        else {
                            // Can't merge non-adjacent blocks
                            break;
                        }
                    }
                }
            }

            // No suitable contiguous free space found
            // Attempt to remove oldest process
            if (!memoryQueue.empty()) {
                removeOldestProcess();
            }
            else {
                // No processes to remove, allocation fails
                return false;
            }
        }
    }
    else {
        // Paging allocation remains unchanged
        unsigned int numPages = (size + memPerFrame - 1) / memPerFrame;
        std::vector<int> freeFrames;

        while (!findFreeFrames(numPages, freeFrames)) {
            if (!memoryQueue.empty()) {
                removeOldestProcess();
            }
            else {
                // No processes to remove, allocation fails
                return false;
            }
        }

        // Allocate frames
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

void MemoryManager::deallocateMemory(Process* process) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    if (flatMemory) {
        bool found = false;

        for (size_t i = 0; i < memoryBlocks.size(); ++i) {
            if (memoryBlocks[i].process == process) {
                usedMemory -= memoryBlocks[i].size;
                memoryBlocks[i].process = nullptr;
                found = true;
                break;  // Assuming process occupies a single block
            }
        }

        if (found) {
            // Merge adjacent free blocks
            mergeAdjacentFreeBlocks();
        }
    }
    else {
        // Paging deallocation remains unchanged
        auto it = pageTables.find(process);
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

    memoryQueue.remove(process);
    swappedOutProcesses.erase(process);
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
        for (const auto& block : memoryBlocks) {
            if (block.process != nullptr) {
                result.emplace_back(block.process, block.size);
            }
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
    return process->isInMemory();
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