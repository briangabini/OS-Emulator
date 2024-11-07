#include "GlobalConfig.h"
#include "PrintCommand.h"
#include "Process.h"
#include "TypedefRepo.h"
#include <string>
#include <utility>

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
    this->commandCounter++;
}

bool Process::isFinished() const {
    return this->commandCounter >= this->commandList.size();
}

int Process::getCommandCounter() const {
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
    return this->currentState;
}

String Process::getName() const {
    return this->name;
}

std::chrono::time_point<std::chrono::system_clock> Process::getCreationTime() const {
    return creationTime;
}

void Process::setState(Process::ProcessState state) {
    if (state == FINISHED && hasMemoryAllocated) {
        deallocateMemory();
    }
    this->currentState = state;
}

void Process::setCpuCoreId(int _cpuCoreId) {
    this->cpuCoreId = _cpuCoreId;
}

bool Process::allocateMemory() {
    if (!hasMemoryAllocated) {
        hasMemoryAllocated = MemoryManager::getInstance()->allocateMemory(name);
    }
    return hasMemoryAllocated;
}

void Process::deallocateMemory() {
    if (hasMemoryAllocated) {
        MemoryManager::getInstance()->deallocateMemory(name);
        hasMemoryAllocated = false;
    }
}

bool Process::hasMemory() const {
    return hasMemoryAllocated;
}