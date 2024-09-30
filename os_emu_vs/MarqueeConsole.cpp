#include "MarqueeConsole.h"

MarqueeConsole::MarqueeConsole()
    : AConsole("MarqueeConsole"), marqueeText("Welcome to the Marquee Console! "), position(0), screenWidth(80) {
}

void MarqueeConsole::onEnabled() {
    display();
    process();
}

void MarqueeConsole::process() {
    while (true) {
        if (_kbhit()) {
           int ch = _getch();
           if (ch == 'q') {
               break;
           }
        }
        updateMarquee();
        display();
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust for refresh rate
    }
}

void MarqueeConsole::display() {
    std::string displayText = marqueeText.substr(position) + marqueeText.substr(0, position);
    std::cout << "\r" << displayText.substr(0, screenWidth) << std::flush;
}

void MarqueeConsole::updateMarquee() {
    position = (position + 1) % marqueeText.length();
}