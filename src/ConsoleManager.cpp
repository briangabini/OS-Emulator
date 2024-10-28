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
    : testing(false), initialized(false), scheduler(nullptr) {
    mainConsole = new MainConsole(*this);
}

ConsoleManager::~ConsoleManager() {
    stopSchedulerTest();
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
    initialized = true;
    return true;
}

bool ConsoleManager::isInitialized() const {
    return initialized;
}

void ConsoleManager::safePrint(const std::string& message) {
    std::lock_guard<std::mutex> lock(ioMutex);
    // Move to new line and print message
    std::cout << "\n" << message << std::endl;
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

void ConsoleManager::createProcess(const std::string& name) {
    std::lock_guard<std::mutex> lock(processMutex);
    if (processes.find(name) == processes.end()) {
        Process* process = new Process(name);
        processes[name] = process;
        scheduler->addProcess(process);
    }
    else {
        std::cout << "Process with name '" << name << "' already exists.\n";
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
    std::cout << "Scheduler test started. Generating dummy processes every " << batchProcessFreq << " CPU cycles...\n";
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
    int processCounter = 1;
    unsigned int freq = config.getBatchProcessFreq();

    while (true) {
        {
            std::unique_lock<std::mutex> lock(testMutex);
            if (!testing) break;
        }

        // Generate a batch of dummy processes
        for (int i = 0; i < 5; ++i) {
            std::string processName = "dummyProcess" + std::to_string(processCounter++);
            createProcess(processName);

            // Add dummy commands to the process
            Process* process = getProcess(processName);
            unsigned int numIns = config.getMinIns() + rand() % (config.getMaxIns() - config.getMinIns() + 1);
            for (unsigned int j = 0; j < numIns; ++j) {
                process->addCommand(new PrintCommand("Hello from " + processName + " Instruction " + std::to_string(j + 1)));
            }

            // std::cout << "Generated dummy process: " << processName << " with " << numIns << " instructions.\n";
        }

        // Wait for 'batchProcessFreq' CPU cycles
        std::unique_lock<std::mutex> lock(testMutex);
        testCV.wait_for(lock, std::chrono::milliseconds(freq), [this]() { return !testing; });
        if (!testing) break;
    }
}

void ConsoleManager::startScheduler10() {
    std::lock_guard<std::mutex> lock(testMutex);
    // Generate 10 processes, each with 100 print commands
    for (int i = 1; i <= 10; ++i) {
        std::string processName = "process" + std::to_string(i);
        createProcess(processName);

        Process* process = getProcess(processName);
        for (int j = 1; j <= 100; ++j) {
            process->addCommand(new PrintCommand("Message " + std::to_string(j) + " from " + processName));
        }

        std::cout << "Generated process: " << processName << " with 100 print commands.\n";
    }
}

std::mutex& ConsoleManager::getIOMutex() {
    return ioMutex;
}
