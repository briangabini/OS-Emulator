#pragma once
#include <iostream>

class MemoryManager
{
public:

	// for singleton pattern
	static MemoryManager* getInstance();
	static void initialize();
	static void destroy();

private:
	MemoryManager();
	~MemoryManager() = default;
	MemoryManager(MemoryManager const&) {};					// copy constructor is private
	MemoryManager& operator=(MemoryManager const&) {};		// assignment operator is private
	static MemoryManager* sharedInstance;
};
