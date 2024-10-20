#pragma once
#include "AScheduler.h"
#include "IETThread.h"
#include "Process.h"
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

// create 2 mode enums kernel and user
enum class Mode {
	KERNEL,
	USER
};

class GlobalScheduler : public IETThread {
public:
	static GlobalScheduler* getInstance();
	static void initialize();
	static void destroy();
	void run() override;

    std::shared_ptr<Process> createProcess(String processName, Mode mode);
	std::shared_ptr<Process> findProcess(String& name) const;

	void test_init100Processes();
	void monitorProcesses() const;
	void logToFile() const;
	void tick() const;


    // for scheduler-test
    void startSchedulerTest();
    void stopSchedulerTest();

private:
    GlobalScheduler();
    ~GlobalScheduler();
    GlobalScheduler(GlobalScheduler const&) {};
    GlobalScheduler& operator=(GlobalScheduler const&) {};

    static GlobalScheduler* sharedInstance;
    std::shared_ptr<AScheduler> scheduler;
	std::unordered_map<String, std::shared_ptr<Process>> processes;

    // for scheduler-test
    bool schedulerTestRunning = false;
    std::thread processGeneratorThread;
    void generateProcesses();
	mutable std::string lastMonitorOutput;

};

std::string formatTimestamp(const std::chrono::time_point<std::chrono::system_clock>& timePoint);
