#pragma once
#include "ICommand.h"
#include "TypedefRepo.h"
#include <chrono>
#include <string>
#include <utility>
#include <vector>

class Process {

public:
	// Some Typedefs
	// For time
	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

	enum ProcessState
	{
		READY,
		RUNNING,
		WAITING,
		FINISHED
	};

	Process(int pid, String name);
	Process() = default;

	void addCommand(ICommand::CommandType commandType);
	void executeCurrentCommand() const;
	void moveToNextLine();

	bool isFinished() const;
	int getCommandCounter() const;
	int getLinesOfCode() const;
	int getPID() const;
	int getCPUCoreID() const;
	ProcessState getState() const;
	String getName() const;
	std::chrono::time_point<std::chrono::system_clock> getCreationTime() const;

	// setters
	void setState(ProcessState state);
	void setCpuCoreId(int _cpuCoreId);

	// week 8
	void setMemoryRequired(int memoryRequired);
	int getMemoryRequired() const;
	void setMemoryPtr(void* ptr);
	void* getMemoryPtr();
	TimePoint getMemoryAllocatedTime();
	void setMemoryAllocatedTime(TimePoint time);
	

private:
	int pid;
	String name;
	using CommandList = std::vector<std::shared_ptr<ICommand>>;
	CommandList commandList;

	int commandCounter;
	int cpuCoreId = -1;
	ProcessState currentState;
	std::chrono::time_point<std::chrono::system_clock> creationTime;

	// RequirementFlags requirements;
	// friend class ResourceEmulator
	friend class FCFSScheduler;

	// week 8
	
	struct MemoryInfo {
		int memoryRequired;
		void* memoryPtr = nullptr;			// Used to check if the process is allocated memory
		TimePoint memoryAllocatedTime;
	} memoryInfo;
};