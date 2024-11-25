#include "PagingAllocator.h"
#include "GlobalConfig.h"

PagingAllocator::PagingAllocator(size_t maxMemorySize)
	: maximumSize(maxMemorySize) 
{
	numFrames = maxMemorySize / GlobalConfig::getInstance()->getMemoryPerFrame();

	for (size_t i = 0; i < numFrames; ++i) {
		freeFramesList.push_back(i);
	}
}

void* PagingAllocator::allocate(size_t size, int processId) {
	// TODO: implement this
	return nullptr;
}

void PagingAllocator::deallocate(void* ptr, size_t size) {
	// TODO: implement
}

std::string PagingAllocator::visualizeMemory() {
	// TODO: implement
	return "";
}

// other methods
size_t PagingAllocator::getExternalFragmentation() const {
	// TODO: implement
	return 0;
}
