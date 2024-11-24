#pragma once
#include "IMemoryAllocator.h"
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

// FlatMemoryAllocator class implementing IMemoryAllocator
class FlatMemoryAllocator : public IMemoryAllocator {
public:
	explicit FlatMemoryAllocator(size_t maximumSize);
	virtual ~FlatMemoryAllocator();
	static FlatMemoryAllocator* getInstance();
	static void initialize();
	static void destroy();


	void* allocate(size_t size, int processId) override;
	void deallocate(void* ptr, size_t size) override;
	std::string visualizeMemory() override;
	int getProcessCount() const;
	size_t getExternalFragmentation() const;

private:
	size_t maximumSize;
	size_t allocatedSize;
	int processCount = 0;

	std::vector<char> memory;
	std::unordered_map<size_t, bool> allocationMap;
	std::unordered_map<size_t, int> processMap;

	void initializeMemory();
	bool canAllocateAt(size_t index, size_t size) const;
	void allocateAt(size_t index, size_t size, int pId);
	void deallocateAt(size_t index, size_t size);
	static FlatMemoryAllocator* sharedInstance;
};
