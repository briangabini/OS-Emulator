#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>

class IMemoryAllocator {
public:
	virtual void* allocate(size_t size) = 0;
	virtual void deallocate(void* ptr) = 0;
	virtual std::string visualizeMemory() = 0;
};
