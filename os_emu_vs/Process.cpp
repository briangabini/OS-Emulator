#include "GlobalConfig.h"
#include "PrintCommand.h"
#include "Process.h"
#include "TypedefRepo.h"
#include "MemoryManager.h"
#include "ConsoleManager.h"
#include <string>
#include <utility>
#include <iostream>

Process::Process(int pid, String name)
    : pid(pid)
    , name(std::move(name))
    , commandCounter(0)
    , currentState(READY)
    , creationTime(std::chrono::system_clock::now()) {
}

void Process::addCommand(ICommand::CommandType commandType) {
    GlobalConfig* config = GlobalConfig::getInstance();

    unsigned int randIns = rand() %
        (config->getMaxIns() - config->getMinIns() + 1) + config->getMinIns();

    for (int i = 0; i < randIns; i++) {
        if (commandType == ICommand::CommandType::PRINT) {
            String message = "Sample text";
            commandList.push_back(std::make_shared<PrintCommand>(pid, message));
        }
    }
}

void Process::executeCurrentCommand() const {
    if (commandCounter >= 0 && static_cast<size_t>(commandCounter) < commandList.size()) {
        this->commandList[this->commandCounter]->execute();
    }
}

void Process::moveToNextLine() {
    std::lock_guard<std::mutex> lock(stateMutex);
    this->commandCounter++;
}

bool Process::isFinished() const {
    std::lock_guard<std::mutex> lock(stateMutex);
    return this->commandCounter >= this->commandList.size() || currentState == FINISHED;
}

int Process::getCommandCounter() const {
    std::lock_guard<std::mutex> lock(stateMutex);
    return this->commandCounter;
}

int Process::getLinesOfCode() const {
    return this->commandList.size();
}

int Process::getPID() const {
    return this->pid;
}

int Process::getCPUCoreID() const {
    return this->cpuCoreId;
}

Process::ProcessState Process::getState() const {
    std::lock_guard<std::mutex> lock(stateMutex);
    return this->currentState;
}

String Process::getName() const {
    return this->name;
}

std::chrono::time_point<std::chrono::system_clock> Process::getCreationTime() const {
    return creationTime;
}

void Process::performCleanup() {
    // Check if already cleaning up
    if (isCleaningUp.exchange(true)) {
        return;
    }

    try {
        // Handle memory cleanup
        if (hasMemoryAllocated) {
            try {
                MemoryManager::getInstance()->deallocateMemory(name);
                hasMemoryAllocated = false;
            }
            catch (const std::exception& e) {
                std::cerr << "Error deallocating memory for process " << name << ": " << e.what() << std::endl;
            }
        }

        // Handle console cleanup
        try {
            auto console = ConsoleManager::getInstance();
            if (console) {
                console->unregisterScreen(name);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error unregistering console screen for process " << name << ": " << e.what() << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error during process cleanup: " << e.what() << std::endl;
    }
}

void Process::setState(Process::ProcessState state) {
    std::lock_guard<std::mutex> lock(stateMutex);

    // Don't change state if it's already FINISHED
    if (currentState == FINISHED) {
        return;
    }

    // Only proceed if we're actually changing state
    if (state == currentState) {
        return;
    }

    // If transitioning to FINISHED state, perform cleanup first
    if (state == FINISHED) {
        performCleanup();
    }

    this->currentState = state;
}

void Process::setCpuCoreId(int _cpuCoreId) {
    this->cpuCoreId = _cpuCoreId;
}

bool Process::allocateMemory() {
    std::lock_guard<std::mutex> lock(stateMutex);
    if (!hasMemoryAllocated) {
        try {
            hasMemoryAllocated = MemoryManager::getInstance()->allocateMemory(name);
        }
        catch (const std::exception& e) {
            std::cerr << "Error allocating memory for process " << name << ": " << e.what() << std::endl;
            return false;
        }
    }
    return hasMemoryAllocated;
}

void Process::deallocateMemory() {
    std::lock_guard<std::mutex> lock(stateMutex);
    if (hasMemoryAllocated) {
        try {
            MemoryManager::getInstance()->deallocateMemory(name);
            hasMemoryAllocated = false;
        }
        catch (const std::exception& e) {
            std::cerr << "Error deallocating memory for process " << name << ": " << e.what() << std::endl;
        }
    }
}

bool Process::hasMemory() const {
    std::lock_guard<std::mutex> lock(stateMutex);
    return hasMemoryAllocated;
}