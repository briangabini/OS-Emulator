#include "ConsoleManager.h"
#include "MainConsole.h"
#include "Screen.h"
#include "Marquee.h"
#include "MarqueeNT.h"
#include "SchedulerFCFS.h"
#include <iostream>

ConsoleManager::ConsoleManager() {
    mainConsole = new MainConsole(*this);
    scheduler = new SchedulerFCFS(4); // Declare 4 cores
    scheduler->start();
}

ConsoleManager::~ConsoleManager() {
    scheduler->stop();
    delete scheduler;
    delete mainConsole;
    for (auto& pair : processes) {
        delete pair.second;
    }
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

std::mutex& ConsoleManager::getIOMutex() {
    return ioMutex;
}
