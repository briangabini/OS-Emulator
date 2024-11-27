#pragma once
#include "IMemoryAllocator.h"
#include <iostream>
#include <string>

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

	int getTotalMemory() const;
	int getUsedMemory() const;
	int getFreeMemory() const;
	int getNumPagedIn() const;
	int getNumPagedOut() const;

	void setTotalMemory(int totalMemory);
	void setUsedMemory(int usedMemory);
	void setFreeMemory(int freeMemory);
	void setNumPagedIn(int numPagedIn);
	void setNumPagedOut(int numPagedOut);

	double getMemoryUtilization();

private:
	MemoryManager();
	~MemoryManager() = default;
	MemoryManager(MemoryManager const&) {};					// copy constructor is private
	MemoryManager& operator=(MemoryManager const&) {};		// assignment operator is private
	static MemoryManager* sharedInstance;

	// assign a memory allocator
	std::shared_ptr<IMemoryAllocator> memoryAllocator;

	struct AdditionalMemoryInfo {
		int totalMemory = 0;		// total memory in KB
		//int usedMemory = 0; 		// total active memory used by processes
		//int freeMemory = 0; 		// total free memory that can still be used by other processes
		int numPagedIn = 0;			// accumulated number of pages that have been paged in
		int numPagedOut = 0;		// accumulated number of pages that have been paged out
	} additionalMemoryInfo;

	// other info
	int processCount = 0;
};
