module;
#include <functional>
#include <future>
#include <mutex>
#include <memory>
#include <print>

module emulation;

namespace chippit {
Emulation::Emulation() 
    : cpu_{std::make_unique<Chip8>()}, graphics_{cpu_.get()}, app_{graphics_, input_}, rom_{cpu_.get()} {
    std::print("Emulation::Emulation()\n");
    opcodeTable_ = {
        std::bind(&Emulation::opcode0X, this),
        std::bind(&Emulation::opcode1X, this),
        std::bind(&Emulation::opcode2X, this),
        std::bind(&Emulation::opcode3X, this),
        std::bind(&Emulation::opcode4X, this),
        std::bind(&Emulation::opcode5X, this),
        std::bind(&Emulation::opcode6X, this),
        std::bind(&Emulation::opcode7X, this),
        {}, {}, {}, {}, {}, {}, {}, {}
    };
}

void Emulation::run() {
    std::print("Emulation::run()\n");
    graphics_.init();
    input_.init();

    cpuThreadFuture_ = std::async(std::launch::async, std::bind(&Emulation::cpu_thread, this));
    app_.run();
    cpuThreadFuture_.wait();
}

void Emulation::reset() {
    cpu_->pc_ = 0x200;
    cpu_->opcode = 0;
    cpu_->I = 0;
    cpu_->sp_ = 0;
}

void Emulation::reset(std::string_view romPath) {
    reset();
    rom_.load(romPath);
}

void Emulation::cpu_thread() {
    while(!finished_.load()) {
        auto now = std::chrono::steady_clock::now();
        if (now - now_ > frequency) {
            cpu_tick();
        }
    }
    reset();
}

void Emulation::cpu_tick() {
    std::unique_lock<std::mutex> lock_{cpuMutex_};
    fetch();
    decodeAndExecute();
    updateTimers();
}

void Emulation::fetch() {
    cpu_->opcode = static_cast<decltype(cpu_->opcode)>(cpu_->memory_[cpu_->pc_] << 8 | cpu_->memory_[cpu_->pc_ + 1]); // Big-endian
}

void Emulation::decodeAndExecute() {
    std::uint8_t const* parts = reinterpret_cast<std::uint8_t const*>(&(cpu_->opcode));
    if(parts[0] < 9u) { // TODO: implement the rest
        auto& func = opcodeTable_.at(parts[0]);
        func();
    } else {
        std::print("Unknown opcode\n");
        cpu_->pc_ += 2;
    }
}

void Emulation::updateTimers() {
    // TODO: update timers
}

void Emulation::opcode0X() {
    switch(cpu_->opcode) {
        case 0x00E0:
        {
            // clear screen
            std::fill(std::begin(cpu_->graphics_), std::end(cpu_->graphics_), std::byte{0});
            cpu_->pc_ += 2;
            break;
        }
        case 0x00EE:
        {
            // return to address pulled from stack
            cpu_->pc_ = cpu_->stack_[cpu_->sp_];
            if(cpu_->sp_ != 0) {
                cpu_->sp_--;
            }
            break;
        }
        default:
        {
            // call 1802 program at 0XNNN
            std::print("0NNN is not implemented, reading the next instruction\n");
            cpu_->pc_ += 2;
            break;
        }
    }
}

void Emulation::opcode1X() {
    std::print("Unknown opcode\n");
    cpu_->pc_ += 2;
}

void Emulation::opcode2X() {
    std::print("Unknown opcode\n");
    cpu_->pc_ += 2;
}

void Emulation::opcode3X() {
    std::print("Unknown opcode\n");
    cpu_->pc_ += 2;
}

void Emulation::opcode4X() {
    std::print("Unknown opcode\n");
    cpu_->pc_ += 2;
}

void Emulation::opcode5X() {
    std::print("Unknown opcode\n");
    cpu_->pc_ += 2;
}

void Emulation::opcode6X() {
    std::print("Unknown opcode\n");
    cpu_->pc_ += 2;
}

void Emulation::opcode7X() {
    std::print("Unknown opcode\n");
    cpu_->pc_ += 2;
}
} // namespace chippit