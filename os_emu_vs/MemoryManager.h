#pragma once
#include "IMemoryAllocator.h"
#include <iostream>

class MemoryManager
{
public:

	// for singleton pattern
	static MemoryManager* getInstance();
	static void initialize();
	static void destroy();

	// other methods
	std::shared_ptr<IMemoryAllocator> getMemoryAllocator() const;
	int getProcessCount() const;
	void setProcessCount(int count);

private:
	MemoryManager();
	~MemoryManager() = default;
	MemoryManager(MemoryManager const&) {};					// copy constructor is private
	MemoryManager& operator=(MemoryManager const&) {};		// assignment operator is private
	static MemoryManager* sharedInstance;

	// assign a memory allocator
	std::shared_ptr<IMemoryAllocator> memoryAllocator;

	// other info
	int processCount = 0;
};
