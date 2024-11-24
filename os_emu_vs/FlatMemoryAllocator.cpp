#include "FlatMemoryAllocator.h"
#include "GlobalConfig.h"
#include <string>
#include <unordered_map>
#include <vector>

FlatMemoryAllocator* FlatMemoryAllocator::sharedInstance = nullptr;
/* PUBLIC METHODS */
FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize)
	: maximumSize(maximumSize), allocatedSize(0), memory(maximumSize, '.')
{
	for (size_t i = 0; i < maximumSize; ++i) {
		allocationMap[i] = false;
		processMap[i] = -1;
	}
}

FlatMemoryAllocator::~FlatMemoryAllocator()
{
	memory.clear();
}

void FlatMemoryAllocator::initialize()
{
	const int maxSize = GlobalConfig::getInstance()->getMaxOverallMemory();

	if (sharedInstance == nullptr) {
		sharedInstance = new FlatMemoryAllocator(maxSize);
	}
}

FlatMemoryAllocator* FlatMemoryAllocator::getInstance() {
	return sharedInstance;
}

void FlatMemoryAllocator::destroy() {
	delete sharedInstance;
	sharedInstance = nullptr;
}

void* FlatMemoryAllocator::allocate(size_t size, int processId)
{
	// Find the first available block that can accommodate the process
	for (size_t i = 0; i < maximumSize - size + 1; ++i) {
		if (!allocationMap[i] && canAllocateAt(i, size)) {
			allocateAt(i, size, processId);
			return &memory[i];
		}
	}

	// No available block found,
	return nullptr;
}

void FlatMemoryAllocator::deallocate(void* ptr, size_t size) {
	// Find the index of the memory block to deallocate
	size_t index = static_cast<char*>(ptr) - &memory[0];
	if (allocationMap[index]) {
		deallocateAt(index, size);
	}
}

int FlatMemoryAllocator::getProcessCount() const {
	return processCount;
}

size_t FlatMemoryAllocator::getExternalFragmentation() const {
	return maximumSize - allocatedSize;
}

std::string FlatMemoryAllocator::visualizeMemory() {
	std::ostringstream oss;

	oss << "----end---- = " << maximumSize << "\n";

	int i = maximumSize - 1;
	while (i > 0) {
		if (allocationMap[i]) {
			oss << i + 1 << "\n";
			oss << "P" << processMap[i] << "\n";
			i -= GlobalConfig::getInstance()->getMemoryPerProcess() - 1;
			oss << i << "\n\n";
		}
		i--;
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
		if (allocationMap.find(i) != allocationMap.end() && allocationMap.at(i)) {
			return false;  // If any part of the block is already allocated, return false
		}
	}
	return true;
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size, int processId) {
	// Mark the memory block as allocated
	for (size_t i = index; i < index + size; ++i) {
		allocationMap[i] = true;
		processMap[i] = processId;
	}
	allocatedSize += size;
	processCount++;
}

void FlatMemoryAllocator::deallocateAt(size_t index, size_t size) {
	// Mark the memory block as deallocated
	for (size_t i = index; i < index + size; ++i) {
		allocationMap[i] = false;
		processMap[i] = -1;
	}
	allocatedSize -= size;
	processCount--;
}