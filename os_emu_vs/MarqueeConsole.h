#pragma once
#include "AConsole.h"
#include <string>
#include <chrono>
#include <thread>
#include <conio.h>
#include <iostream>


class MarqueeConsole : public AConsole
{
public:
	MarqueeConsole();
	~MarqueeConsole() override = default;
	void onEnabled() override;
	void process() override;
	void display() override;
private:
	std::string marqueeText;
	int position;
	int screenWidth;
	void updateMarquee();
};
