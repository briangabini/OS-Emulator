#pragma once
#include <mutex>
#include <queue>
#include <unordered_map>
#include "IMemoryAllocator.h"
#include "Process.h"

class PagingAllocator : public IMemoryAllocator {
public:
	static const int UNALLOCATED_FRAME = -1;

	PagingAllocator(size_t maxMemorySize);

	std::string visualizeMemory() override;

	// other methods
	void* allocate(std::shared_ptr<Process> process) override;
	void deallocate(std::shared_ptr<Process> process) override;

	int getUsedMemory() const override;
	int getFreeMemory() const override;

private:
	size_t maximumSize;			// maxMemorySize
	size_t allocatedSize;

	size_t numFrames;
	std::unordered_map<int, int> frameMap;
	std::queue<size_t> freeFrameList;
	std::mutex allocatorMutex;

	// new vars and methods
	size_t allocateFrames(size_t numFrames, size_t processId);
	void deallocateFrames(size_t frameIndex);

	mutable std::mutex mtx;
};
