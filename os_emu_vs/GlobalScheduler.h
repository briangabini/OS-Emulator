#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <atomic>
#include <thread>
#include "AScheduler.h"
#include "Process.h"
#include "IETThread.h"

class GlobalScheduler : public IETThread {
public:
	static GlobalScheduler* getInstance();
    static void initialize();
    static void destroy();
	void run() override;

    std::shared_ptr<Process> createUniqueProcess(String processName);
	std::shared_ptr<Process> findProcess(String& name) const;

    // Week 6
    void test_init10Processes();

    void listProcesses() const;

    void create10DummyProcesses();
    void monitorProcesses() const;
    bool allProcessesFinished() const;

	void tick() const;

private:
    GlobalScheduler();
    ~GlobalScheduler() = default;
    GlobalScheduler(GlobalScheduler const&) {};
    GlobalScheduler& operator=(GlobalScheduler const&) {};

    static GlobalScheduler* sharedInstance;
    std::shared_ptr<AScheduler> scheduler;
	std::unordered_map<String, std::shared_ptr<Process>> processes;


};

std::string formatTimestamp(const std::chrono::time_point<std::chrono::system_clock>& timePoint);
