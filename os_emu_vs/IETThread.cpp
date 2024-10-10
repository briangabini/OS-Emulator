#include <thread>

#include "IETThread.h"

IETThread::IETThread() : running(false) {}

IETThread::~IETThread()
{
	if (thread.joinable())
	{
		running = false;
		thread.join();
	}
}

void IETThread::start() {
	running = true;
	thread = std::thread(&IETThread::run, this);
}

void IETThread::sleep(int timeInMillis) {
	std::this_thread::sleep_for(std::chrono::milliseconds(timeInMillis));
}
