#pragma once
#include "ICommand.h"
#include "TypedefRepo.h"
#include "MemoryManager.h"
#include <chrono>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

class Process {
public:
    enum ProcessState {
        READY,
        RUNNING,
        WAITING,
        FINISHED
    };

    Process(int pid, String name);
    Process() = default;
    ~Process() = default;

    // Command management
    void addCommand(ICommand::CommandType commandType);
    void executeCurrentCommand() const;
    void moveToNextLine();
    bool isFinished() const;

    // Getters
    int getCommandCounter() const;
    int getLinesOfCode() const;
    int getPID() const;
    int getCPUCoreID() const;
    ProcessState getState() const;
    String getName() const;
    std::chrono::time_point<std::chrono::system_clock> getCreationTime() const;

    // Setters
    void setState(ProcessState state);
    void setCpuCoreId(int _cpuCoreId);

    // Memory management
    bool allocateMemory();
    void deallocateMemory();
    bool hasMemory() const;

private:
    int pid;
    String name;
    typedef std::vector<std::shared_ptr<ICommand>> CommandList;
    CommandList commandList;

    int commandCounter;
    int cpuCoreId = -1;
    ProcessState currentState;
    std::chrono::time_point<std::chrono::system_clock> creationTime;

    bool hasMemoryAllocated = false;
    mutable std::mutex stateMutex;  // Single mutex for all state-related operations
    std::atomic<bool> isCleaningUp{ false };

    // Private helper methods
    void performCleanup();

    friend class FCFSScheduler;
};