#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Process.h"

class IMemoryAllocator {
public:
	virtual void* allocate(std::shared_ptr<Process> process) = 0;
	virtual void deallocate(std::shared_ptr<Process> process) = 0;
	virtual std::string visualizeMemory() = 0;

	// add new methods
	virtual int getUsedMemory() const = 0;
	virtual int getFreeMemory() const = 0;
};