#include "Process.h"
#include <iostream>

Process::Process(const std::string& name)
    : name(name), currentLine(0), isRunning(true) {
    creationTime = std::chrono::system_clock::now();
    totalLines = 10; // Simulated total lines of code
    for (int i = 1; i <= totalLines; ++i) {
        codeLines.push_back("Code line " + std::to_string(i));
    }
    processThread = std::thread(&Process::run, this);
}

Process::~Process() {
    isRunning = false;
    if (processThread.joinable()) {
        processThread.join();
    }
}

void Process::run() {
    while (isRunning && currentLine < totalLines) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        {
            std::lock_guard<std::mutex> lock(mutex);
            ++currentLine;
        }
    }
}

std::string Process::getName() const {
    return name;
}

std::time_t Process::getCreationTime() const {
    return std::chrono::system_clock::to_time_t(creationTime);
}

int Process::getCurrentLine() const {
    std::lock_guard<std::mutex> lock(mutex);
    return currentLine;
}

int Process::getTotalLines() const {
    return totalLines;
}

std::string Process::getCurrentCodeLine() const {
    std::lock_guard<std::mutex> lock(mutex);
    if (currentLine < totalLines) {
        return codeLines[currentLine];
    }
    else {
        return "Process has completed execution.";
    }
}

bool Process::isCompleted() const {
    std::lock_guard<std::mutex> lock(mutex);
    return currentLine >= totalLines;
}
