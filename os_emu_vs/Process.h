#pragma once
#include <string>
#include <chrono>

class Process {
public:
    explicit Process(const std::string& name);
	Process() = default;

    std::string getName() const;
    std::string getTimestamp() const;
    int getCurrentLine() const;
    int getTotalLines() const;

private:
    std::string name;
    std::string timestamp;
    int currentLine;
    int totalLines;

    std::string getCurrentTimestamp() const;
};