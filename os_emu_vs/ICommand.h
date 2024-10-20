#pragma once
// #include "GlobalConfig.h"
#include "IETThread.h"

class ICommand
{
public:
	enum CommandType
	{
		IO,
		PRINT
	};

	ICommand(int pid, CommandType commandType);
	CommandType getCommandType() const;
	virtual void execute(int cpuCoreId);

protected:
	int pid;
	CommandType commandType;
};

inline ICommand::CommandType ICommand::getCommandType() const
{
	return this->commandType;
}

inline ICommand::ICommand(int pid, CommandType commandType)
{
	this->pid = pid;
	this->commandType = commandType;
}

inline void ICommand::execute(int cpuCoreId)
{
	// Do nothing
}
