#include <iostream>
#include <thread>
#include <chrono>
#include <print>
#include <cmdlime/commandlinereader.h>

import emulation;

namespace args {
struct Cfg : public cmdlime::Config {
    CMDLIME_ARG(romPath, std::string) << "Path to a CHIP-8 ROM binary";
};
} // namespace

int main(int argc, char** argv) {
    auto reader = cmdlime::CommandLineReader{"chippit"};
    auto cfg = reader.read<args::Cfg>(argc, argv);

    chippit::Emulation emu{};
    emu.reset(cfg.romPath);
    emu.run();
    return 0;
}