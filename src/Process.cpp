#include "Process.h"
#include "Command.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <mutex>

int Process::nextId = 1;
bool Process::loggingEnabled = false;

Process::Process(const std::string& name)
    : name(name), currentLine(0), totalLines(0), completed(false),
      memorySize(0), inMemory(false) {
    creationTime = std::chrono::system_clock::now();
    id = nextId++;

    if (loggingEnabled) {
        // Initialize process log file only if logging is enabled
        std::ofstream logfile(name + ".txt", std::ios::out);
        if (logfile.is_open()) {
            logfile << "Process name: " << name << "\nLogs:\n";
            logfile.close();
        }
        else {
            std::cerr << "Unable to create log file for process " << name << std::endl;
        }
    }
}

Process::~Process() {
    while (!commandQueue.empty()) {
        delete commandQueue.front();
        commandQueue.pop();
    }
}

int Process::getId() const {
    return id;
}

const std::string& Process::getName() const {
    return name;
}

void Process::setMemorySize(unsigned int size) {
    memorySize = size;
}

unsigned int Process::getMemorySize() const {
    return memorySize;
}

void Process::setInMemory(bool inMemory) {
    this->inMemory = inMemory;
}

bool Process::isInMemory() const {
    return inMemory;
}

void Process::addCommand(Command* cmd) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        commandQueue.push(cmd);
    }

    // Store command description for display
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        totalLines++;
        codeLines.push_back(cmd->getDescription());
    }
}

Command* Process::getNextCommand() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (commandQueue.empty()) {
        return nullptr;
    }
    Command* cmd = commandQueue.front();
    commandQueue.pop();
    return cmd;
}

void Process::log(const std::string& message, int coreId) {
    if (!loggingEnabled) return;

    std::lock_guard<std::mutex> lock(logMutex);
    // Open the process's log file and append the message
    std::ofstream logfile(name + ".txt", std::ios::app);

    if (logfile.is_open()) {
        // Get current time
        std::time_t now = std::time(nullptr);
        std::tm now_tm;

        localtime_s(&now_tm, &now);

        logfile << "(" << std::put_time(&now_tm, "%m/%d/%Y %I:%M:%S%p") << ") ";
        logfile << "Core:" << coreId << " \"" << message << "\"\n";
        logfile.close();
    }
    else {
        std::cerr << "Unable to open log file for process " << name << std::endl;
    }
}

std::time_t Process::getCreationTime() const {
    return std::chrono::system_clock::to_time_t(creationTime);
}

int Process::getCurrentLine() const {
    std::lock_guard<std::mutex> lock(stateMutex);
    return currentLine;
}

int Process::getTotalLines() const {
    std::lock_guard<std::mutex> lock(stateMutex);
    return totalLines;
}

std::string Process::getCurrentCodeLine() const {
    std::lock_guard<std::mutex> lock(stateMutex);
    if (currentLine > 0 && currentLine <= totalLines) {
        return codeLines[currentLine - 1];
    }
    else {
        return "No code line is currently being executed.";
    }
}

bool Process::isCompleted() const {
    std::lock_guard<std::mutex> lock(stateMutex);
    return completed;
}

void Process::incrementCurrentLine() {
    std::lock_guard<std::mutex> lock(stateMutex);
    currentLine++;
}

void Process::setCompleted(bool value) {
    std::lock_guard<std::mutex> lock(stateMutex);
    completed = value;
}

void Process::resetCompleted() {
    std::lock_guard<std::mutex> lock(stateMutex);
    completed = false;
}

void Process::setLoggingEnabled(bool enabled) {
    loggingEnabled = enabled;
}

bool Process::isLoggingEnabled() {
    return loggingEnabled;
}
