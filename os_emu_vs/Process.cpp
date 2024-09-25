// Process.cpp
#include "Process.h"
#include <iomanip>
#include <sstream>
#include <ctime>

Process::Process(const std::string& name)
    : name(name), currentLine(1), totalLines(100), timestamp(getCurrentTimestamp()) {}

Process::Process() : currentLine(1), totalLines(100), timestamp(getCurrentTimestamp()) {}

std::string Process::getName() const {
    return name;
}

std::string Process::getTimestamp() const {
    return timestamp;
}

int Process::getCurrentLine() const {
    return currentLine;
}

int Process::getTotalLines() const {
    return totalLines;
}

std::string Process::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;

#ifdef _WIN32
    localtime_s(&local_tm, &now_time);
#else
    localtime_r(&now_time, &local_tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%m/%d/%Y, %I:%M:%S %p");
    return oss.str();
}