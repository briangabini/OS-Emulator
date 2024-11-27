#pragma once
#include "AScheduler.h"
#include "IETThread.h"
#include "Process.h"
#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <thread>

// create 2 mode enums kernel and user
enum class Mode {
	KERNEL,
	USER
};

class GlobalScheduler : public IETThread {
public:
	struct CompareByProcessNumber;

	static GlobalScheduler* getInstance();
	static void initialize();
	static void destroy();
	void run() override;

	std::shared_ptr<Process> createProcess(String processName, Mode mode);
	std::shared_ptr<Process> findProcess(String& name) const;
	std::shared_ptr<Process> findProcessById(int id) const;

	void monitorProcesses() const;
	void logToFile() const;
	void logMemory() const;
	void tick() const;


	// for scheduler-test
	void startSchedulerTest();
	void stopSchedulerTest();

	// for cpu ticks
	void startCpuThread();
	void stopCpuThread();
	int getCpuCycles() const;

	// for active cpu ticks
	void startActiveCpuThread();
	void stopActiveCpuThread();
	int getActiveCpuCycles() const;

	double getCpuUtilization() const;
	const std::map<String, std::shared_ptr<Process>, CompareByProcessNumber>* getProcesses() const;

private:
	GlobalScheduler(SchedulingAlgorithm algo);
	~GlobalScheduler();
	GlobalScheduler(GlobalScheduler const&) {};
	GlobalScheduler& operator=(GlobalScheduler const&) {};

	static GlobalScheduler* sharedInstance;
	std::shared_ptr<AScheduler> scheduler;

	struct CompareByProcessNumber {
		bool operator()(const String& lhs, const String& rhs) const {
			auto extractNumber = [](const String& str) {
				size_t pos = str.find('_');
				if (pos != String::npos) {
					try {
						return std::stoi(str.substr(pos + 1));
					}
					catch (const std::invalid_argument& e) {
						return -1;
					}
					catch (const std::out_of_range& e) {
						return -1;
					}
				}
				return -1;
				};

			int lhsNumber = extractNumber(lhs);
			int rhsNumber = extractNumber(rhs);

			if (lhsNumber != -1 && rhsNumber != -1) {
				return lhsNumber < rhsNumber;
			}
			else if (lhsNumber == -1 && rhsNumber == -1) {
				return lhs < rhs;
			}
			else {
				return lhsNumber != -1;
			}
		}
	};

	std::map<String, std::shared_ptr<Process>, CompareByProcessNumber> processes;

	// for scheduler-test
	bool schedulerTestRunning = false;
	std::thread processGeneratorThread;
	void generateProcesses();
	mutable std::string lastMonitorOutput;

	// cpuTicks
	std::atomic<int> cpuCycles;
	std::atomic<bool> cpuCycleRunning;
	std::thread cpuThread;

	// active cpu ticks
	std::atomic<int> activeCpuCycles;
	std::atomic<bool> activeCpuCycleRunning;
	std::thread activeCpuThread;
};

std::string formatTimestamp(const std::chrono::time_point<std::chrono::system_clock>& timePoint);
