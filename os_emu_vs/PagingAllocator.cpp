#include "PagingAllocator.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include "GlobalConfig.h"
#include "MemoryManager.h"

PagingAllocator::PagingAllocator(size_t maxMemorySize)
	: maximumSize(maxMemorySize) 
{
	numFrames = maxMemorySize / GlobalConfig::getInstance()->getMemoryPerFrame();

	for (size_t i = 0; i < numFrames; ++i) {
		freeFrameList.push_back(i);

		// initialize the unordered map with frames
		frameMap[i] = -1;
	}
}

// other methods
size_t PagingAllocator::getExternalFragmentation() const {
	return freeFrameList.size() * GlobalConfig::getInstance()->getMemoryPerFrame();
}

// methods from lecture
void* PagingAllocator::allocate(std::shared_ptr<Process> process) {
	size_t processId = process->getPID();
	size_t numFramesNeeded = process->getNumberOfPages();

	// without backing store
	if (numFramesNeeded > freeFrameList.size()) {
		std::cerr << "Memory allocation failed. Not enough free frames.\n";
		return nullptr;
	}

	// Allocate frames for the process
	size_t frameIndex = allocateFrames(numFramesNeeded, processId);
	void* memoryPtr = reinterpret_cast<void*>(frameIndex);
	process->setMemoryPtr(memoryPtr);


	// increment processCount
	int currentProcessCount = MemoryManager::getInstance()->getProcessCount();
	MemoryManager::getInstance()->setProcessCount(currentProcessCount + 1);

	return memoryPtr;
}

void PagingAllocator::deallocate(std::shared_ptr<Process> process) {
	size_t processId = process->getPID();
	std::vector<size_t> framesToDeallocate;

	// Find frames allocated to the process
	for (const auto& entry : frameMap) {
		if (entry.second == processId) {
			framesToDeallocate.push_back(entry.first);
		}
	}

	if (framesToDeallocate.size() == 0) {
		std::cerr << "Memory deallocation failed. Process not found.\n";
		return;
	}

	// Deallocate the frames
	for (size_t frameIndex : framesToDeallocate) {
		deallocateFrames(frameIndex);
	}

	process->setMemoryPtr(nullptr);
	// decrement processCount
	int currentProcessCount = MemoryManager::getInstance()->getProcessCount();
	MemoryManager::getInstance()->setProcessCount(currentProcessCount + 1);
}

std::string PagingAllocator::visualizeMemory() {
	std::ostringstream oss;

	oss << "Memory visualization:\n";
	for (size_t frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
		auto it = frameMap.find(frameIndex);
		if (it != frameMap.end() && it->second != -1) {
			oss << "Frame " << frameIndex << " -> Process " << it->second << "\n";
		}
		else {
			oss << "Frame " << frameIndex << " -> Free\n";
		}
	} 
	oss << "---------------------\n";

	return oss.str();
}

size_t PagingAllocator::allocateFrames(size_t numFrames, size_t processId) {
	std::vector<size_t> allocatedFrames;

	// Collect the required number of frames from the freeFrameList
	for (size_t i = 0; i < numFrames; ++i) {
		allocatedFrames.push_back(freeFrameList.back());
		freeFrameList.pop_back();
	}

	// Map allocated frames to the process ID
	for (size_t frameIndex : allocatedFrames) {
		frameMap[frameIndex] = processId;
	}

	// Return the index of the first allocated frame
	return allocatedFrames.front();
}

void PagingAllocator::deallocateFrames(size_t frameIndex) {
	// Set frame to -1 to "deallocate"
	frameMap[frameIndex] = -1;

	// Add frame to the free frame list
	freeFrameList.push_back(frameIndex);
}

