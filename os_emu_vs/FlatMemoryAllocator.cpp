#include "FlatMemoryAllocator.h"
#include <string>
#include <unordered_map>
#include <vector>
#include "GlobalConfig.h"
#include "MemoryManager.h"
#include "GlobalScheduler.h"

/* PUBLIC METHODS */
FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize)
	: maximumSize(maximumSize), allocatedSize(0), memory(maximumSize, '.')
{
	this->initializeMemory();
}

FlatMemoryAllocator::~FlatMemoryAllocator()
{
	memory.clear();
}

void* FlatMemoryAllocator::allocate(std::shared_ptr<Process> process)
{
	std::lock_guard<std::mutex> lock(mtx);

	int size = process->getMemoryRequired();
	int processId = process->getPID();


	// Find the first available block that can accommodate the process
	for (size_t i = 0; i < maximumSize - size + 1; ++i) {
		if (!allocationMap[i] && canAllocateAt(i, size)) {
			allocateAt(i, size, processId);
			process->setMemoryPtr(&memory[i]);

			// get current time
			process->setMemoryAllocatedTime(Process::TimePoint(std::chrono::system_clock::now()));
			return &memory[i];				// return the pointer to the i-th element of the memory vector
		}
	}

	// No available block found,
	return nullptr;
}

void FlatMemoryAllocator::deallocate(std::shared_ptr<Process> process) {
	// Find the index of the memory block to deallocate
	std::lock_guard<std::mutex> lock(mtx);
	size_t index = static_cast<char*>(process->getMemoryPtr()) - &memory[0];
	if (allocationMap[index]) {
		deallocateAt(index, process->getMemoryRequired());
	}

	// set memoryAllocatedTime to 0
	process->setMemoryAllocatedTime(Process::TimePoint());
	process->setMemoryPtr(nullptr);
}

std::string FlatMemoryAllocator::visualizeMemory() {
	std::lock_guard<std::mutex> lock(mtx);

	std::ostringstream oss;

	oss << "----end---- = " << maximumSize << "\n";

	int i = maximumSize - 1;
	while (i > 0) {
		if (allocationMap[i]) {
			oss << i + 1 << "\n";
			oss << "P" << processMap[i] << "\n";

			// get process using id
			std::shared_ptr<Process> process = GlobalScheduler::getInstance()->findProcessById(processMap[i]);

			i -= process->getMemoryRequired()  - 1;
			oss << i << "\n\n";
		}
		--i;
	}

	// Print lower boundary
	oss << "----start---- = 0\n";

	return oss.str();
}

/* PRIVATE METHODS */
void FlatMemoryAllocator::initializeMemory() {
	std::fill(memory.begin(), memory.end(), '.');	// '.' represents unallocated memory
	allocationMap.clear();
	for (size_t i = 0; i < maximumSize; ++i) {
		allocationMap[i] = false;
		processMap[i] = -1;
	}
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) const {
	// Check if the memory block is large enough
	if (index + size > maximumSize) return false;
	for (size_t i = index; i < index + size; ++i) {
		if (allocationMap.contains(i) && allocationMap.at(i)) {	// Checks if the key exists (meaning it is properly allocated) and if the value is true
			return false;  // If any part of the block is already allocated, return false
		}
	}
	return true;
}

// allocate memory block by setting the allocationMap to true, and processMap to processId
void FlatMemoryAllocator::allocateAt(size_t index, size_t size, int processId) {
	// Mark the memory block as allocated
	for (size_t i = index; i < index + size; ++i) {
		allocationMap[i] = true;
		processMap[i] = processId;
	}
	allocatedSize += size;

	// increment processCount
	int currentProcessCount = MemoryManager::getInstance()->getProcessCount();
	MemoryManager::getInstance()->setProcessCount(currentProcessCount + 1);
}

// deallocate memory block by setting the allocationMap to false, and processMap to -1
void FlatMemoryAllocator::deallocateAt(size_t index, size_t size) {
	// Mark the memory block as deallocated
	for (size_t i = index; i < index + size; ++i) {
		allocationMap[i] = false;
		processMap[i] = -1;
	}
	allocatedSize -= size;

	// decrement processCount
	int currentProcessCount = MemoryManager::getInstance()->getProcessCount();
	MemoryManager::getInstance()->setProcessCount(currentProcessCount - 1);

}

int FlatMemoryAllocator::getUsedMemory() const {
	return allocatedSize;
}

int FlatMemoryAllocator::getFreeMemory() const {
	return maximumSize - allocatedSize;
}