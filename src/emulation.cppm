module;
#include <atomic>
#include <functional>
#include <memory>
#include <chrono>
#include <future>
#include <print>

export module emulation;

import application;
import core;
import graphics;
import input;
import rom;

namespace chippit {
/// @brief Main emulation class
export class Emulation {
public:
    Emulation() : cpu_{std::make_unique<Chip8>()}, graphics_{cpu_.get()}, app_{graphics_, input_} {
        std::print("Emulation::Emulation()\n");
    }

    void run() {
        std::print("Emulation::run()");
        graphics_.init();
        input_.init();
        rom_.init();

        cpuThreadFuture_ = std::async(std::launch::async, std::bind(&Emulation::cpu_thread, this));
        app_.run();
        cpuThreadFuture_.wait();
    }

    void cpu_thread() {
        while(!finished_.load()) {
            // TODO: update cpu
        }
    }
private:
    std::unique_ptr<Chip8> cpu_; // TODO: maybe some factory for this
    Graphics graphics_;
    Input input_;
    Rom rom_;
    Application app_;
    

    std::future<void> cpuThreadFuture_;
    std::chrono::time_point<std::chrono::steady_clock> now_{};

    std::atomic_bool finished_{false};
};
} // namespace chippit