#include "MemoryManager.h"

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
}
