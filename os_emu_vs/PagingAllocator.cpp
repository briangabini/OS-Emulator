#include "PagingAllocator.h"

PagingAllocator::PagingAllocator(size_t maxMemorySize)
	: maximumSize(maxMemorySize) {}

void* PagingAllocator::allocate(size_t size, int processId) {
	// TODO: implement this
	return nullptr;
}

void PagingAllocator::deallocate(void* ptr, size_t size) {
	// TODO: implement
}

void PagingAllocator::initializeMemory() {
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
