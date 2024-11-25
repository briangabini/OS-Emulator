#pragma once
#include "IMemoryAllocator.h"
#include "Process.h"

class PagingAllocator : public IMemoryAllocator {
public:
	static const int UNALLOCATED_FRAME = -1;

	PagingAllocator(size_t maxMemorySize);

	std::string visualizeMemory() override;

	// other methods
	size_t getExternalFragmentation() const override;

	void* allocate(std::shared_ptr<Process> process) override;
	void deallocate(std::shared_ptr<Process> process) override;

private:
	size_t maximumSize;			// maxMemorySize
	size_t allocatedSize;

	std::vector<char> memory;
	//std::unordered_map<size_t, bool> allocationMap;
	//std::unordered_map<size_t, int> processMap;

	size_t numFrames;
	std::unordered_map<int, int> frameMap;
	std::vector<size_t> freeFrameList;

	// new vars and methods
	size_t allocateFrames(size_t numFrames, size_t processId);
	void deallocateFrames(size_t frameIndex);
};
