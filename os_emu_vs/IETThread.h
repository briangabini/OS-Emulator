#pragma once
#include <thread>
#include <atomic>

class IETThread
{
public:
	IETThread();
	virtual ~IETThread();
	void start();
	static void sleep(int ms);
	virtual void run() = 0;

protected:
	std::atomic<bool> running;

private:
	std::thread thread;
};

