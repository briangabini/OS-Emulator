#include "PagingAllocator.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include "GlobalConfig.h"

PagingAllocator::PagingAllocator(size_t maxMemorySize)
	: maximumSize(maxMemorySize) 
{
	numFrames = maxMemorySize / GlobalConfig::getInstance()->getMemoryPerFrame();

	for (size_t i = 0; i < numFrames; ++i) {
		freeFrameList.push_back(i);
	}
}

// other methods
size_t PagingAllocator::getExternalFragmentation() const {
	// TODO: implement
	return 0;
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
	return reinterpret_cast<void*>(frameIndex);
}

void PagingAllocator::deallocate(std::shared_ptr<Process> process) {
	size_t processId = process->getPID();

	// Find frames allocated to the process and deallocate
	auto it = std::find_if(frameMap.begin(), frameMap.end(),
		[processId](const auto& entry) { return entry.second == processId; });

	while (it != frameMap.end()) {
		size_t frameIndex = it->first;
		deallocateFrames(1, frameIndex);
		it = std::find_if(frameMap.begin(), frameMap.end(), [processId](const auto& entry) { return entry.second == processId;  });
	}
}

std::string PagingAllocator::visualizeMemory() {
	std::ostringstream oss;

	oss << "Memory visualization:\n";
	for (size_t frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
		auto it = frameMap.find(frameIndex);
		if (it != frameMap.end()) {
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
	size_t frameIndex = freeFrameList.back();
	freeFrameList.pop_back();

	// Map allocated frames to the process ID
	for (size_t i = 0; i < numFrames; ++i) {
		frameMap[frameIndex + i] = processId;
	}

	return frameIndex;
}

void PagingAllocator::deallocateFrames(size_t numFrames, size_t frameIndex) {
	// Remove mapping of deallocated frames
	for (size_t i = 0; i < numFrames; ++i) {
		frameMap.erase(frameIndex + i);
	}

	// Add frames to the free frame list
	for (size_t i = 0; numFrames; ++i) {
		freeFrameList.push_back(frameIndex + i);
	}
}
