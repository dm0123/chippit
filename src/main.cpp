#include <iostream>
#include <thread>
#include <chrono>
#include <print>

import emulation;

int main(int /*argc*/, char** /*argv*/) {
    std::print("main\n");
    chippit::Emulation emu{};
    emu.run();
    return 0;
}