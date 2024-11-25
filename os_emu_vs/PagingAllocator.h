#pragma once
#include "IMemoryAllocator.h"
#include "Process.h"

class PagingAllocator : public IMemoryAllocator {
public:
	PagingAllocator(size_t maxMemorySize);

	void* allocate(size_t size, int processId) override;
	void deallocate(void* ptr, size_t size) override;
	std::string visualizeMemory() override;

	// other methods
	size_t getExternalFragmentation() const override;

private:
	size_t maximumSize;
	size_t allocatedSize;

	std::vector<char> memory;
	std::unordered_map<size_t, bool> allocationMap;
	std::unordered_map<size_t, int> processMap;

	void initializeMemory() override;
	/*bool canAllocateAt(size_t index, size_t size) const;
	void allocateAt(size_t index, size_t size, int pId);
	void deallocateAt(size_t index, size_t size);*/
};
