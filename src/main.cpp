#include <iostream>
#include <thread>
#include <chrono>
#include <print>
#include <cmdlime/commandlinereader.h>

import emulation;

namespace args {
struct Cfg : public cmdlime::Config {
    CMDLIME_ARG(romPath, std::string) << "Path to a CHIP-8 ROM binary";
    CMDLIME_FLAG(cpuDump) << "Dump CPU state";
    CMDLIME_FLAG(memoryDump) << "Dump memory";
    CMDLIME_FLAG(graphicsDump) << "Dump graphics";
    CMDLIME_EXITFLAG(help);
};
} // namespace

int main(int argc, char** argv) {
    auto reader = cmdlime::CommandLineReader{"chippit"};
    auto cfg = args::Cfg{};
    try {
        cfg = reader.read<args::Cfg>(argc, argv);
    }
    catch(cmdlime::Error const e) {
        std::cerr << e.what() << "\n";
        std::cout << reader.usageInfo<args::Cfg>();
    }

    if(cfg.help || cfg.romPath.empty()) {
        std::cout << reader.usageInfoDetailed<args::Cfg>();
        return 0;
    }

    chippit::Emulation emu{cfg.cpuDump, cfg.memoryDump, cfg.graphicsDump};
    emu.reset(cfg.romPath);
    emu.run();
    return 0;
}