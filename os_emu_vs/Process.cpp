#include "Process.h"
#include <string>

std::string Process::getName() const {
	return name;
}

int Process::getCurrentLine() const {
	return currentLine;
}

int Process::getTotalLines() const {
	return totalLines;
}

std::ostream& operator<<(std::ostream& out, const Process& process) {
	out << "Screen - Process: " << process.getName() << "\n";
	out << "Instruction: " << process.getCurrentLine() << " / " << process.getTotalLines() << "\n";

	return out;
}


// constructor
Process::Process(const std::string& name)
	: name{ name }, currentLine{ 0 }, totalLines{ 0 }
{
}
