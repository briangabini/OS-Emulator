#include "ConsoleManager.h"
#include "MainConsole.h"
#include "Screen.h"
#include "Marquee.h"
#include "MarqueeNT.h"
#include "SchedulerFCFS.h"
#include "Command.h"
#include <iostream>
#include <thread>
#include <chrono>

ConsoleManager::ConsoleManager()
    : testing(false) {
    mainConsole = new MainConsole(*this);
    scheduler = new SchedulerFCFS(4, *this); // Declare 4 cores
}

ConsoleManager::~ConsoleManager() {
    stopSchedulerTest();
    scheduler->stop();
    delete scheduler;
    delete mainConsole;
    for (auto& pair : processes) {
        delete pair.second;
    }
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
    mainConsole->run();
}

void ConsoleManager::switchToScreen(Process* process) {
    Screen screen(*this, process);
    screen.run();
}

void ConsoleManager::switchToMarquee() {
    Marquee marquee(*this);
    marquee.run();
}

void ConsoleManager::switchToMarqueeNT() {
    MarqueeNT marqueeNT(*this);
    marqueeNT.run();
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
    testing = true;
    testThread = std::thread(&ConsoleManager::schedulerTestLoop, this);
    std::cout << "Scheduler test started.\n";
}

void ConsoleManager::stopSchedulerTest() {
    {
        std::lock_guard<std::mutex> lock(testMutex);
        if (!testing) {
            std::cout << "Scheduler test is not running.\n";
            return;
        }
        testing = false;
    }
    testCV.notify_all();
    if (testThread.joinable()) {
        testThread.join();
    }
    std::cout << "Scheduler test stopped.\n";
}

void ConsoleManager::schedulerTestLoop() {
    int processCounter = 1;
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
            process->addCommand(new PrintCommand("Hello from " + processName));

            std::cout << "Generated dummy process: " << processName << "\n";
        }

        // Wait for some time before generating the next batch
        std::unique_lock<std::mutex> lock(testMutex);
        testCV.wait_for(lock, std::chrono::seconds(5), [this]() { return !testing; });
        if (!testing) break;
    }
}

void ConsoleManager::startScheduler10() {
    std::lock_guard<std::mutex> lock(testMutex);
    // Generate 10 processes, each with 100 print commands
    for (int i = 1; i <= 10; ++i) {
        std::string processName = "process" + std::to_string(i);
        createProcess(processName);

        // Add 100 print commands to the process
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
