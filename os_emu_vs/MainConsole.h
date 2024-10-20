#pragma once
#include "AConsole.h"
#include <array>
#include <string_view>
#include <iostream>


class MainConsole : public AConsole
{
public:
	MainConsole() : AConsole("MainConsole") {}
	~MainConsole() override = default;
	void onEnabled() override;
	void process() override;
	void display() override;

};

