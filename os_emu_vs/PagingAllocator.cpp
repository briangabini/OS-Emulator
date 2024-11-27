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
		freeFrameList.push(i);

		// initialize the unordered map with frames
		frameMap[i] = UNALLOCATED_FRAME;
	}
}

// methods from lecture
void* PagingAllocator::allocate(std::shared_ptr<Process> process) {
	std::lock_guard<std::mutex> lock(mtx); // Lock for thread safety

	size_t processId = process->getPID();
	size_t numFramesNeeded = process->getNumberOfPages();

	// print pid and numFramesNeeded
	//std::cout << "Process ID: " << processId << " Number of Frames Needed: " << numFramesNeeded << std::endl;
	//std::cout << "Allocating " << numFramesNeeded << " frames for process " << processId << std::endl;

	if (numFramesNeeded > freeFrameList.size()) {
		//std::cout << "numFramesNeeded > freeFrameList.size()";
		return nullptr;
	}

	// Allocate frames for the process
	size_t frameIndex = allocateFrames(numFramesNeeded, processId);
	void* memoryPtr = &frameMap[frameIndex];
	process->setMemoryPtr(memoryPtr);

	// increment processCount
	int currentProcessCount = MemoryManager::getInstance()->getProcessCount();
	MemoryManager::getInstance()->setProcessCount(currentProcessCount + 1);

	return memoryPtr;
}

void PagingAllocator::deallocate(std::shared_ptr<Process> process) {
	std::lock_guard<std::mutex> lock(mtx);
	size_t processId = process->getPID();
	std::vector<size_t> framesToDeallocate;

	// Find frames allocated to the process
	for (const auto& entry : frameMap) {
		if (entry.second == processId) {
			framesToDeallocate.push_back(entry.first);
		}
	}

	if (framesToDeallocate.size() == 0) {
		return;
	}

	// Deallocate the frames
	for (size_t frameIndex : framesToDeallocate) {
		deallocateFrames(frameIndex);
	}

	// list deallocated frames
	/*for (size_t frameIndex : framesToDeallocate) {
		std::cout << "Deallocated Frame: " << frameIndex << std::endl;
	}*/

	process->setMemoryPtr(nullptr);
	// decrement processCount
	int currentProcessCount = MemoryManager::getInstance()->getProcessCount();
	MemoryManager::getInstance()->setProcessCount(currentProcessCount - 1);
}

std::string PagingAllocator::visualizeMemory() {
	std::lock_guard<std::mutex> lock(mtx);
	std::ostringstream oss;

	oss << "Memory visualization:\n";
	for (size_t frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
		auto it = frameMap.find(frameIndex);
		if (it != frameMap.end() && it->second != UNALLOCATED_FRAME) {
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
		allocatedFrames.push_back(freeFrameList.front());
		freeFrameList.pop();
		
		// increment numPagedIn
		auto memoryManager = MemoryManager::getInstance();
		int currentNumPagedIn = memoryManager->getNumPagedIn();
		memoryManager->setNumPagedIn(currentNumPagedIn + 1);
	}

	// print allocated frames
	/*for (size_t frameIndex : allocatedFrames) {
		std::cout << "Allocated Frame: " << frameIndex << std::endl;
	}*/

	// Map allocated frames to the process ID
	for (size_t frameIndex : allocatedFrames) {
		frameMap[frameIndex] = processId;
	}

	// Return the index of the first allocated frame
	return allocatedFrames.front();
}

void PagingAllocator::deallocateFrames(size_t frameIndex) {
	// Set frame to UNALLOCATED_FRAME to "deallocate"
	frameMap[frameIndex] = UNALLOCATED_FRAME;

	// Add frame to the free frame list
	freeFrameList.push(frameIndex);

	// increment numPagedIn
	auto memoryManager = MemoryManager::getInstance();
	int currentNumPagedOut = memoryManager->getNumPagedOut();
	memoryManager->setNumPagedOut(currentNumPagedOut + 1);
}

int PagingAllocator::getUsedMemory() const {
	std::lock_guard<std::mutex> lock(mtx);
	int usedMemory = 0;
	size_t memoryPerFrame = GlobalConfig::getInstance()->getMemoryPerFrame();

	for (const auto& entry : frameMap) {
		if (entry.second != UNALLOCATED_FRAME) {
			usedMemory += memoryPerFrame;
		}
	}

	return usedMemory;
}

int PagingAllocator::getFreeMemory() const {
	std::lock_guard<std::mutex> lock(mtx);
	int freeMemory = 0;
	size_t memoryPerFrame = GlobalConfig::getInstance()->getMemoryPerFrame();

	for (const auto& entry : frameMap) {
		if (entry.second == UNALLOCATED_FRAME) {
			freeMemory += memoryPerFrame;
		}
	}

	return freeMemory;
}
