#pragma once
#include "IMemoryAllocator.h"
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include "Process.h"

// FlatMemoryAllocator class implementing IMemoryAllocator
class FlatMemoryAllocator : public IMemoryAllocator {
public:
	explicit FlatMemoryAllocator(size_t maximumSize);
	virtual ~FlatMemoryAllocator();

	void* allocate(std::shared_ptr<Process> process) override;
	void deallocate(std::shared_ptr<Process> process) override;
	std::string visualizeMemory() override;
	size_t getExternalFragmentation() const override;

private:
	size_t maximumSize;
	size_t allocatedSize;

	std::vector<char> memory;
	std::unordered_map<size_t, bool> allocationMap;
	std::unordered_map<size_t, int> processMap;

	void initializeMemory();
	bool canAllocateAt(size_t index, size_t size) const;
	void allocateAt(size_t index, size_t size, int pId);
	void deallocateAt(size_t index, size_t size);
};
