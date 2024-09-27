#pragma once
#include <string>
#include "TypedefRepo.h"

class AConsole
{
public:
	AConsole() = default;
	explicit AConsole(const String& name);
	virtual ~AConsole() = default;

	String getName() const;

	virtual void onEnabled() = 0;	// called when the screen is shown the first time
	virtual void display() = 0;		// called per frame
	virtual void process() = 0;     // for input processing

	String name;
	friend class ConsoleManager;
};
