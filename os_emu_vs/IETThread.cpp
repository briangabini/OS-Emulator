#include <thread>

#include "IETThread.h"


void IETThread::start() {
	std::thread(&IETThread::run, this).detach();
}

void IETThread::sleep(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}