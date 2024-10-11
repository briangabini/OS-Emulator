#include "PrintCommand.h"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

PrintCommand::PrintCommand(int pid, String& toPrint) : ICommand(pid, PRINT) {
	this->toPrint = toPrint;
}

void PrintCommand::execute(int cpuCoreId) {
	//ICommand::execute();

	// Get the current time
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm now_tm;

#ifdef _WIN32
	localtime_s(&now_tm, &now_time);
#else
	localtime_r(&now_tm, &now_time);
#endif

	// Get the CPU core ID (assuming a method getCPUCoreID exists in Process)
	//int cpuCoreID = -1; // Replace with actual method to get CPU core ID

	// Format the message
	std::ostringstream oss;
	oss << "(" << std::put_time(&now_tm, "%m/%d/%Y %I:%M:%S%p") << ") "
		<< "Core:" << cpuCoreId << " \"" << this->toPrint << "\"\n";

	// Log to file
	logToFile(oss.str());
}

void PrintCommand::logToFile(const String& message) {
	// Create a file name based on the process ID
	String fileName = "process_" + std::to_string(this->pid) + "_log.txt";

	// Open the file in append mode
	std::ofstream outFile(fileName, std::ios_base::app);

	outFile.seekp(0, std::ios::end);

	if (outFile.is_open()) {
		// If the file is empty, write the header
		if (outFile.tellp() == 0) {
			outFile << "====\n";
			outFile << "Process name: " << "process_" << this->pid << "\n";
			outFile << "Logs:\n\n";
		}
		outFile << message;
		outFile.close();
	}
	else {
		std::cerr << "Unable to open file: " << fileName << '\n';
	}
}
