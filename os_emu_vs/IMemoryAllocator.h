#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>

class IMemoryAllocator {
public:
	virtual void* allocate(size_t size, int processId) = 0;
	virtual void deallocate(void* ptr, size_t size) = 0;
	virtual void initializeMemory() = 0;
	virtual std::string visualizeMemory() = 0;

	// add new methods
	virtual size_t getExternalFragmentation() const = 0;
};
