#include "GlobalConfig.h"
#include "PrintCommand.h"
#include "Process.h"

Process::Process(int pid, String name)
	:
	pid(pid), name(std::move(name)),
	commandCounter(0),
	currentState(ProcessState::READY),
	creationTime(std::chrono::system_clock::now()),
	memoryInfo{ GlobalConfig::getInstance()->getMemoryPerProcess(), nullptr, {}, -1}
{
	if (!GlobalConfig::getInstance()->isUsingFlatMemoryAllocator()) {
		memoryInfo.numberOfPages = GlobalConfig::getInstance()->generateRandomNumberOfPages();
	}
}

void Process::addCommand(ICommand::CommandType commandType)
{
	GlobalConfig* config = GlobalConfig::getInstance();

	unsigned int randIns = rand() %
		(config->getMaxIns() - config->getMinIns() + 1) + config->getMinIns();

	for (int i = 0; i < randIns; i++)
	{
		if (commandType == ICommand::CommandType::PRINT) {
			String message = "Sample text";
			commandList.push_back(std::make_shared<PrintCommand>(pid, message));
		}
	}
}

void Process::executeCurrentCommand() const
{
	this->commandList[this->commandCounter]->execute();
}

void Process::moveToNextLine()
{
	this->commandCounter++;
}

bool Process::isFinished() const
{
	return this->commandCounter == this->commandList.size();
}

int Process::getCommandCounter() const
{
	return this->commandCounter;
}

int Process::getLinesOfCode() const
{
	return this->commandList.size();
}

int Process::getPID() const
{
	return this->pid;
}

int Process::getCPUCoreID() const
{
	return this->cpuCoreId;
}

Process::ProcessState Process::getState() const
{
	return this->currentState;
}

String Process::getName() const
{
	return this->name;
}

std::chrono::time_point<std::chrono::system_clock> Process::getCreationTime() const {
	return creationTime;
}

void Process::setState(Process::ProcessState state) {
	this->currentState = state;
}

void Process::setCpuCoreId(int _cpuCoreId) {
	this->cpuCoreId = _cpuCoreId;
}

void Process::setMemoryRequired(int memoryRequired) {
	this->memoryInfo.memoryRequired = memoryRequired;
}

int Process::getMemoryRequired() const {
	return memoryInfo.memoryRequired;
}

void* Process::getMemoryPtr() {
	return memoryInfo.memoryPtr;
}

Process::TimePoint Process::getMemoryAllocatedTime()
{
	return memoryInfo.memoryAllocatedTime;
}

void Process::setMemoryAllocatedTime(Process::TimePoint time)
{
	memoryInfo.memoryAllocatedTime = time;
}

void Process::setMemoryPtr(void* ptr) {
	memoryInfo.memoryPtr = ptr;
}

void Process::setNumberOfPages(int numberOfPages)
{
	memoryInfo.numberOfPages = numberOfPages;
}

int Process::getNumberOfPages() const {
	return memoryInfo.numberOfPages;
}