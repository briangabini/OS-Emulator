#pragma once
class IETThread
{
public:
	IETThread() {};
	~IETThread() = default;

	void start();
	static void sleep(int ms);
protected:
	virtual void run() = 0;
};

