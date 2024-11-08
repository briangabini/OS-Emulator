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
	}
}

FlatMemoryAllocator::~FlatMemoryAllocator()
{
	memory.clear();
}

void FlatMemoryAllocator::initialize()
{
	int maxSize = GlobalConfig::getInstance()->getMaxOverallMemory();

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

void* FlatMemoryAllocator::allocate(size_t size)
{
	// Find the first available block that can accommodate the process
	for (size_t i = 0; i < maximumSize - size + 1; ++i) {
		if (!allocationMap[i] && canAllocateAt(i, size)) {
			allocateAt(i, size);
			return &memory[i];
		}
	}
	processCount++;

	// No available block found,
	return nullptr;
}

void FlatMemoryAllocator::deallocate(void* ptr) {
	// Find the index of the memory block to deallocate
	size_t index = static_cast<char*>(ptr) - &memory[0];
	if (allocationMap[index]) {
		deallocateAt(index);
		processCount--;
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
	oss << "----end---- = \n" << maximumSize;
	size_t placeholder = maximumSize;
	int j = 0;

	for (size_t i = 0; i < memory.size(); i+= GlobalConfig::getInstance()->getMemoryPerProcess()) {
		if (allocationMap[i]) {
			oss << "P" << j + 1 << "\n";
			placeholder -= GlobalConfig::getInstance()->getMemoryPerProcess();
			oss << placeholder << "\n";
		}
		j++;
		//else {
		//	placeholder -= GlobalConfig::getInstance()->getMemoryPerFrame();  // Assuming each block of free space is 4096 bytes
		//	oss << placeholder << "\n";
		//} 
	}
	oss << "----start---- = 0" << std::endl;

	return oss.str();

}

/* PRIVATE METHODS */
void FlatMemoryAllocator::initializeMemory() {
	std::fill(memory.begin(), memory.end(), '.');	// '.' represents unallocated memory
	for (size_t i = 0; i < maximumSize; ++i) {
		allocationMap[i] = false;
	}
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) const {
	// Check if the memory block is large enough
	return (index + size <= maximumSize);
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size) {
	// Mark the memory block as allocated
	for (size_t i = index; i < index + size; ++i) {
		allocationMap[i] = true;
	}
	allocatedSize += size;
}

void FlatMemoryAllocator::deallocateAt(size_t index) {
	// Mark the memory block as deallocated
	allocationMap[index] = false;
}