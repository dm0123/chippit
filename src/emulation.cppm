module;
#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
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
    static constexpr auto frequency = std::chrono::seconds{1} / 60;
public:
    Emulation();

    void run();
    void reset(std::string_view romPath);
    void reset();

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
private:
    std::unique_ptr<Chip8> cpu_; // TODO: maybe some factory for this
    Graphics graphics_;
    Input input_;
    Rom rom_;
    Application app_;
    

    std::future<void> cpuThreadFuture_;
    std::chrono::time_point<std::chrono::steady_clock> now_{std::chrono::steady_clock::now()};

    std::atomic_bool finished_{false};

    std::array<std::function<void()>, 16> opcodeTable_;
    std::mutex cpuMutex_;

    void fetch();
    void decodeAndExecute();
    void updateTimers();
};
} // namespace chippit