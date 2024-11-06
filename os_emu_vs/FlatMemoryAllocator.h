#pragma once
#include "IMemoryAllocator.h"
#include <vector>
#include <unordered_map>
#include <string>

// FlatMemoryAllocator class implementing IMemoryAllocator
class FlatMemoryAllocator : public IMemoryAllocator {
public: 
	explicit FlatMemoryAllocator(size_t maximumSize);
	virtual ~FlatMemoryAllocator();

	void* allocate(size_t size) override;
	void deallocate(void* ptr) override;
	std::string visualizeMemory() override;

private:
	size_t maximumSize;
	size_t allocatedSize;
	std::vector<char> memory;
	std::unordered_map<size_t, bool> allocationMap;

	void initializeMemory();
	bool canAllocateAt(size_t index, size_t size) const;
	void allocateAt(size_t index, size_t size);
	void deallocateAt(size_t index);
};
