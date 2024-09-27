#include "AConsole.h"
#include "TypedefRepo.h"


AConsole::AConsole(const String& name) : name(name)
{
}

String AConsole::getName() const
{
	return this->name;
}
