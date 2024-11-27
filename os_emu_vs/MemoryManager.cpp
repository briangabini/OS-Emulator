#include "MemoryManager.h"
#include <string>
#include "GlobalConfig.h"
#include "FlatMemoryAllocator.h"
#include "PagingAllocator.h"

// initialize the shared instance
MemoryManager* MemoryManager::sharedInstance = nullptr;

MemoryManager* MemoryManager::getInstance()
{
	return sharedInstance;
}

void MemoryManager::initialize() {
	if (sharedInstance == nullptr) {
		sharedInstance = new MemoryManager();
	}
}

void MemoryManager::destroy()
{
	delete sharedInstance;
	sharedInstance = nullptr;
}

MemoryManager::MemoryManager()
{
	bool usingFlatMemoryAllocator = GlobalConfig::getInstance()->isUsingFlatMemoryAllocator();
	const int maxSize = GlobalConfig::getInstance()->getMaxOverallMemory();
	additionalMemoryInfo.totalMemory = maxSize;

	if (usingFlatMemoryAllocator) {
		memoryAllocator = std::make_shared<FlatMemoryAllocator>(maxSize);
	}
	else {
		memoryAllocator = std::make_shared<PagingAllocator>(maxSize);
	}
}

/* GETTERS AND SETTERS */ 
std::shared_ptr<IMemoryAllocator> MemoryManager::getMemoryAllocator() const {
	return memoryAllocator;
}

int MemoryManager::getProcessCount() const
{
	return processCount;
}

void MemoryManager::setProcessCount(int count) {
	processCount = count;
}

int MemoryManager::getTotalMemory() const {
	return additionalMemoryInfo.totalMemory;
}

int MemoryManager::getUsedMemory() const {
	return memoryAllocator->getUsedMemory();
}

int MemoryManager::getFreeMemory() const {
	return memoryAllocator->getFreeMemory();
}

int MemoryManager::getNumPagedIn() const {
	return additionalMemoryInfo.numPagedIn;
}

int MemoryManager::getNumPagedOut() const {
	return additionalMemoryInfo.numPagedOut;
}

void MemoryManager::setTotalMemory(int totalMemory) {
	additionalMemoryInfo.totalMemory = totalMemory;
}


void MemoryManager::setNumPagedIn(int numPagedIn) {
	additionalMemoryInfo.numPagedIn = numPagedIn;
}

void MemoryManager::setNumPagedOut(int numPagedOut) {
	additionalMemoryInfo.numPagedOut = numPagedOut;
}

std::string MemoryManager::getMemoryUtilization() {
	// usedMemory / totalMemory * 100
	int usedMemory = getUsedMemory();
	int totalMemory = getTotalMemory();
	double utilization = (double)usedMemory / totalMemory * 100;

	return std::to_string(utilization);
}