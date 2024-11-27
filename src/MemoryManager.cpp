#include "MemoryManager.h"
#include "Config.h"
#include <algorithm>
#include <climits>

MemoryManager::MemoryManager()
    : maxMemory(0), memPerFrame(0), totalFrames(0),
    flatMemory(true), usedMemory(0),
    numPagedIn(0), numPagedOut(0),
    idleCpuTicks(0), activeCpuTicks(0), totalCpuTicks(0) {
}

MemoryManager::~MemoryManager() {
}

void MemoryManager::initialize(unsigned int maxMem, unsigned int memPerFrame) {
    maxMemory = maxMem;
    this->memPerFrame = memPerFrame;
    totalFrames = maxMemory / memPerFrame;
    flatMemory = (maxMemory == memPerFrame);
}

bool MemoryManager::allocateMemory(Process* process, unsigned int size) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    if (flatMemory) {
        // Flat memory allocation
        if (usedMemory + size <= maxMemory) {
            usedMemory += size;
            processMemoryMap[process] = size;
            memoryQueue.push_back(process);
            process->setInMemory(true);
            return true;
        }
        else {
            // Need to swap out oldest processes until there's enough memory
            while (usedMemory + size > maxMemory && !memoryQueue.empty()) {
                Process* oldestProcess = memoryQueue.front();
                memoryQueue.pop_front();
                unsigned int freedSize = processMemoryMap[oldestProcess];
                usedMemory -= freedSize;
                processMemoryMap.erase(oldestProcess);
                oldestProcess->setInMemory(false);

                // Note: In a real system, we'd write the process's state to backing store here
            }

            if (usedMemory + size <= maxMemory) {
                usedMemory += size;
                processMemoryMap[process] = size;
                memoryQueue.push_back(process);
                process->setInMemory(true);
                return true;
            }
            else {
                // Not enough memory even after swapping out
                return false;
            }
        }
    }
    else {
        // Paging allocation
        unsigned int numPages = (size + memPerFrame - 1) / memPerFrame;

        if (processPageMap.size() + numPages <= totalFrames) {
            processPageMap[process] = numPages;
            memoryQueue.push_back(process);
            process->setInMemory(true);
            numPagedIn += numPages;
            return true;
        }
        else {
            // Need to swap out oldest processes
            while (processPageMap.size() + numPages > totalFrames && !memoryQueue.empty()) {
                Process* oldestProcess = memoryQueue.front();
                memoryQueue.pop_front();
                unsigned int pagesFreed = processPageMap[oldestProcess];
                processPageMap.erase(oldestProcess);
                oldestProcess->setInMemory(false);
                numPagedOut += pagesFreed;
            }

            if (processPageMap.size() + numPages <= totalFrames) {
                processPageMap[process] = numPages;
                memoryQueue.push_back(process);
                process->setInMemory(true);
                numPagedIn += numPages;
                return true;
            }
            else {
                // Not enough frames even after swapping out
                return false;
            }
        }
    }
}

void MemoryManager::deallocateMemory(Process* process) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    if (flatMemory) {
        auto it = processMemoryMap.find(process);
        if (it != processMemoryMap.end()) {
            usedMemory -= it->second;
            processMemoryMap.erase(it);
        }

        memoryQueue.remove(process);
    }
    else {
        auto it = processPageMap.find(process);
        if (it != processPageMap.end()) {
            numPagedOut += it->second;
            processPageMap.erase(it);
        }

        memoryQueue.remove(process);
    }

    process->setInMemory(false);
}

unsigned int MemoryManager::getTotalMemory() const {
    return maxMemory;
}

unsigned int MemoryManager::getUsedMemory() const {
    std::lock_guard<std::mutex> lock(memoryMutex);

    if (flatMemory) {
        return usedMemory;
    }
    else {
        // Cast to larger type before multiplication to prevent overflow
        unsigned long long totalUsed = static_cast<unsigned long long>(processPageMap.size()) * memPerFrame;
        if (totalUsed > UINT_MAX) {
            return UINT_MAX;  // Return maximum possible value if overflow would occur
        }
        return static_cast<unsigned int>(totalUsed);
    }
}

unsigned int MemoryManager::getFreeMemory() const {
    return getTotalMemory() - getUsedMemory();
}

double MemoryManager::getMemoryUtilization() const {
    return ((double)getUsedMemory() / getTotalMemory()) * 100.0;
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

std::vector<std::pair<Process*, unsigned int>> MemoryManager::getProcessesInMemory() const {
    std::lock_guard<std::mutex> lock(memoryMutex);
    std::vector<std::pair<Process*, unsigned int>> processes;

    if (flatMemory) {
        for (const auto& entry : processMemoryMap) {
            processes.emplace_back(entry.first, entry.second);
        }
    }
    else {
        for (const auto& entry : processPageMap) {
            processes.emplace_back(entry.first, entry.second * memPerFrame);
        }
    }

    return processes;
}

bool MemoryManager::isProcessInMemory(Process* process) const {
    std::lock_guard<std::mutex> lock(memoryMutex);

    if (flatMemory) {
        return processMemoryMap.find(process) != processMemoryMap.end();
    }
    else {
        return processPageMap.find(process) != processPageMap.end();
    }
}

bool MemoryManager::isPaging() const {
    return !flatMemory;
}