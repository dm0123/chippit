module;
#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <chrono>
#include <future>
#include <print>
#include <cstdint>
#include <string_view>

export module emulation;

import application;
import core;
import graphics;
import input;
import rom;

namespace chippit {
/// @brief Main emulation class
export class Emulation {
    static constexpr auto frequency = std::chrono::milliseconds{16};
public:
    Emulation(bool cpuDump, bool memoryDump, bool graphicsDump);

    void run();
    void reset(std::string_view romPath);
    void reset();
    void stop();

    void cpu_thread();

    void cpu_tick();

    void opcode0X();
    void opcode1X();
    void opcode2X();
    void opcode3X();
    void opcode4X();
    void opcode5X();
    void opcode6X();
    void opcode7X();
    void opcode8X();
    void opcode9X();
    void opcodeAX();
    void opcodeBX();
    void opcodeCX();
    void opcodeDX();
    void opcodeEX();
    void opcodeFX();
private:
    std::unique_ptr<Chip8> cpu_; // TODO: maybe some factory for this
    Graphics graphics_;
    Input input_;
    Rom rom_;
    Application app_;
    

    std::future<void> cpuThreadFuture_;

    std::atomic_bool finished_{false};

    std::array<std::function<void()>, 16> opcodeTable_;
    std::mutex cpuMutex_;
    std::random_device rd_;

    bool cpuDump_{false};
    bool memoryDump_{false};
    bool graphicsDump_{false};

    void fetch();
    void decodeAndExecute();
    void updateTimers();

    void dump();
};
} // namespace chippit