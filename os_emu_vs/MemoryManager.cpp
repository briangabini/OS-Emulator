#include "MemoryManager.h"
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
