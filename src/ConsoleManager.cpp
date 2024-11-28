#include "ConsoleManager.h"
#include "MainConsole.h"
#include "Screen.h"
#include "SchedulerFCFS.h"
#include "SchedulerRR.h"
#include "PrintCommand.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>

ConsoleManager::ConsoleManager()
    : testing(false), initialized(false), scheduler(nullptr), cpuCycles(0), cpuCycleRunning(false) {
    mainConsole = new MainConsole(*this);
}

ConsoleManager::~ConsoleManager() {
    stopSchedulerTest();
    stopCpuCycleCounter();
    if (scheduler) {
        scheduler->stop();
        delete scheduler;
    }
    delete mainConsole;
    for (auto& pair : processes) {
        delete pair.second;
    }
}

bool ConsoleManager::initialize() {
    Config& config = Config::getInstance();
    if (!config.loadConfig("config.txt")) {
        std::cerr << "Failed to load configuration." << std::endl;
        return false;
    }

    memoryManager.initialize(
        config.getMaxOverallMem(),
        config.getMemPerFrame()
    );

    if (config.getSchedulerType() == "fcfs") {
        scheduler = new SchedulerFCFS(config.getNumCpu(), *this);
    }
    else if (config.getSchedulerType() == "rr") {
        scheduler = new SchedulerRR(config.getNumCpu(), config.getQuantumCycles(), *this);
    }
    else {
        std::cerr << "Unknown scheduler type in configuration." << std::endl;
        return false;
    }

    startScheduler();
    startCpuCycleCounter();
    initialized = true;
    return true;
}

bool ConsoleManager::isInitialized() const {
    return initialized;
}

void ConsoleManager::startCpuCycleCounter() {
    std::lock_guard<std::mutex> lock(cpuCycleMutex);
    if (cpuCycleRunning) return;
    cpuCycleRunning = true;
    cpuCycleThread = std::thread(&ConsoleManager::cpuCycleLoop, this);
}

void ConsoleManager::stopCpuCycleCounter() {
    {
        std::lock_guard<std::mutex> lock(cpuCycleMutex);
        if (!cpuCycleRunning) return;
        cpuCycleRunning = false;
    }
    cpuCycleCV.notify_all();
    if (cpuCycleThread.joinable()) {
        cpuCycleThread.join();
    }
}

void ConsoleManager::cpuCycleLoop() {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(cpuCycleMutex);
            if (!cpuCycleRunning) break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        cpuCycles++;
    }
}

void ConsoleManager::safePrint(const std::string& message) {
    std::lock_guard<std::mutex> lock(ioMutex);
    // Move to new line and print message
    std::cout << message << std::endl;
    // Reprint the prompt
    std::cout << currentPrompt;
    std::cout.flush();
}

void ConsoleManager::printPrompt() {
    std::lock_guard<std::mutex> lock(ioMutex);
    std::cout << currentPrompt;
    std::cout.flush();
}

void ConsoleManager::setCurrentPrompt(const std::string& prompt) {
    std::lock_guard<std::mutex> lock(ioMutex);
    currentPrompt = prompt;
}


void ConsoleManager::start() {
    mainConsole->run();
}

void ConsoleManager::switchToMainConsole() {
    system("CLS");
    mainConsole->run();
}

void ConsoleManager::switchToScreen(Process* process) {
    system("CLS");
    Screen screen(*this, process);
    screen.run();
}

bool ConsoleManager::createProcess(const std::string& name) {
    std::lock_guard<std::mutex> lock(processMutex);
    if (processes.find(name) == processes.end()) {
        Process* process = new Process(name);

        // Set memory size for the process
        Config& config = Config::getInstance();
        unsigned int minMem = config.getMinMemPerProc();
        unsigned int maxMem = config.getMaxMemPerProc();
        unsigned int memSize = minMem + rand() % (maxMem - minMem + 1);

        // Validate memory size against total available memory
        if (memSize > config.getMaxOverallMem()) {
            std::cout << "Process memory requirement (" << memSize
                << " KB) exceeds system memory ("
                << config.getMaxOverallMem() << " KB).\n";
            delete process;
            return false;
        }

        process->setMemorySize(memSize);

        // Try to allocate memory for the process
        try {
            if (memoryManager.allocateMemory(process, memSize)) {
                processes[name] = process;
                scheduler->addProcess(process);
                return true;
            }
            else {
                // Not enough memory, cannot create process
                delete process;
                std::cout << "Not enough memory to create process '" << name
                    << "' (required: " << memSize << " KB).\n";
                return false;
            }
        }
        catch (const std::exception& e) {
            delete process;
            std::cout << "Error allocating memory for process '" << name
                << "': " << e.what() << "\n";
            return false;
        }
    }
    else {
        std::cout << "Process with name '" << name << "' already exists.\n";
        return false;
    }
}

Process* ConsoleManager::getProcess(const std::string& name) {
    std::lock_guard<std::mutex> lock(processMutex);
    auto it = processes.find(name);
    if (it != processes.end()) {
        return it->second;
    }
    else {
        std::cout << "No process found with name '" << name << "'.\n";
        return nullptr;
    }
}

std::map<std::string, Process*>& ConsoleManager::getProcesses() {
    return processes;
}

MemoryManager& ConsoleManager::getMemoryManager() {
    return memoryManager;
}

Scheduler* ConsoleManager::getScheduler() {
    return scheduler;
}

void ConsoleManager::startScheduler() {
    if (scheduler->isRunning()) {
        std::cout << "Scheduler is already running.\n";
        return;
    }
    scheduler->start();
    std::cout << "Scheduler started.\n";
}

void ConsoleManager::stopScheduler() {
    if (!scheduler) {
        std::cout << "Scheduler is not initialized.\n";
        return;
    }
    if (!scheduler->isRunning()) {
        std::cout << "Scheduler is not running.\n";
        return;
    }
    scheduler->stop();
    std::cout << "Scheduler stopped.\n";
}

void ConsoleManager::pauseScheduler() {
    if (!scheduler->isRunning()) {
        std::cout << "Scheduler is not running.\n";
        return;
    }
    if (scheduler->isPaused()) {
        std::cout << "Scheduler is already paused.\n";
        return;
    }
    scheduler->pause();
    std::cout << "Scheduler paused.\n";
}

void ConsoleManager::resumeScheduler() {
    if (!scheduler->isRunning()) {
        std::cout << "Scheduler is not running.\n";
        return;
    }
    if (!scheduler->isPaused()) {
        std::cout << "Scheduler is not paused.\n";
        return;
    }
    scheduler->resume();
    std::cout << "Scheduler resumed.\n";
}

void ConsoleManager::startSchedulerTest() {
    std::lock_guard<std::mutex> lock(testMutex);
    if (testing) {
        std::cout << "Scheduler test is already running.\n";
        return;
    }
    if (!scheduler->isRunning()) {
        std::cout << "Scheduler is not running. Starting scheduler.\n";
        startScheduler();
    }
    testing = true;
    testThread = std::thread(&ConsoleManager::schedulerTestLoop, this);

    int batchProcessFreq = Config::getInstance().getBatchProcessFreq();
    std::cout << "Scheduler test started. Generating dummy processes every " + std::to_string(batchProcessFreq) + " CPU cycles...\n";
}

void ConsoleManager::stopSchedulerTest() {
    {
        std::lock_guard<std::mutex> lock(testMutex);
        if (!testing) {
            std::cout << "Scheduler test is not running.\n";
            return;
        }
        testing = false;
        testCV.notify_all();
    }
    if (testThread.joinable()) {
        testThread.join();
        std::cout << "Scheduler test stopped.\n";
    }
}

void ConsoleManager::schedulerTestLoop() {
    Config& config = Config::getInstance();
    unsigned int freq = config.getBatchProcessFreq();
    unsigned int nextProcessCycle = cpuCycles.load() + freq;

    while (true) {
        {
            std::unique_lock<std::mutex> lock(testMutex);
            if (!testing) break;
        }

        // Wait until cpuCycles >= nextProcessCycle
        while (cpuCycles.load() < nextProcessCycle) {
            {
                std::unique_lock<std::mutex> lock(testMutex);
                if (!testing) return;
            }
        }

        generateTestProcess("dummyProcess");

        nextProcessCycle += freq;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ConsoleManager::generateTestProcess(const std::string& baseName, std::stringstream* outputStream) {
    // Use atomic increment to get unique process number
    int processNum = processCounter++;
    std::string processName = baseName + std::to_string(processNum);

    if (createProcess(processName)) {
        Process* process = getProcess(processName);
        if (process) {
            Config& config = Config::getInstance();
            unsigned int numIns = config.getMinIns() + rand() % (config.getMaxIns() - config.getMinIns() + 1);

            for (unsigned int j = 0; j < numIns; ++j) {
                process->addCommand(new PrintCommand("Hello from " + processName + " Instruction " + std::to_string(j + 1)));
            }

            // Only output if we're in batch mode (-p flag)
            if (outputStream) {
                *outputStream << "Generated process: " << processName << " with " << numIns << " print commands.\n";
            }
        }
    }
    else if (outputStream) {
        *outputStream << "Failed to create process '" << processName << "'. Skipping...\n";
    }
}

void ConsoleManager::startSchedulerTestWithProcesses(int numProcesses) {
    std::stringstream outputBuffer;
    std::cout << "Generating " << numProcesses << " processes...\n";

    for (int i = 0; i < numProcesses; ++i) {
        generateTestProcess("process", &outputBuffer);
    }

    // Print all process generation messages at once
    std::cout << outputBuffer.str();
}

void ConsoleManager::startSchedulerTestWithDuration(int seconds) {
    std::cout << "Starting scheduler test for " + std::to_string(seconds) + " seconds...\n";
    startSchedulerTest();

    std::thread([this, seconds]() {
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        stopSchedulerTest();
        safePrint("Scheduler test completed after " + std::to_string(seconds) + " seconds.");
        }).detach();
}

std::mutex& ConsoleManager::getIOMutex() {
    return ioMutex;
}