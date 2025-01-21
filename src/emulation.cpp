module;
#include <cstddef>
#include <functional>
#include <future>
#include <limits>
#include <mutex>
#include <memory>
#include <random>
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
        std::bind(&Emulation::opcode8X, this),
        std::bind(&Emulation::opcode9X, this),
        std::bind(&Emulation::opcodeAX, this),
        std::bind(&Emulation::opcodeBX, this),
        std::bind(&Emulation::opcodeCX, this),
        std::bind(&Emulation::opcodeDX, this),
        std::bind(&Emulation::opcodeEX, this),
        std::bind(&Emulation::opcodeFX, this)
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
    cpu_->opcode = 0u;
    cpu_->I = 0u;
    cpu_->sp_ = 0u;
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
    cpu_->opcode = std::to_integer<uint16_t>(cpu_->memory_[cpu_->pc_]) << 8 | std::to_integer<uint16_t>(cpu_->memory_[cpu_->pc_ + 1]); // Big-endian
}

void Emulation::decodeAndExecute() {
    std::uint16_t high = (cpu_->opcode >> 8) >> 4;
    if(high > opcodeTable_.size()) {
        std::print("Unknown opcode {:#06x}\n", cpu_->opcode);
        cpu_->pc_ += 2;
        return;
    }
    auto& func = opcodeTable_.at(high);
    func();
}

void Emulation::updateTimers() {
    if(cpu_->delay_timer_ > 0u) {
        --cpu_->delay_timer_;
    }

    if(cpu_->sound_timer_ > 0u) {
        if(cpu_->sound_timer_ == 1u) {
            std::print("BEEEP!\n"); // TODO: make sound
        }
        --cpu_->sound_timer_;
    }
}

void Emulation::opcode0X() {
    switch(cpu_->opcode) {
        case 0x00E0:
        {
            // clear screen
            std::fill(std::begin(cpu_->graphics_), std::end(cpu_->graphics_), std::byte{0});
            cpu_->pc_ += 2u;
            break;
        }
        case 0x00EE:
        {
            // return to address pulled from stack
            cpu_->pc_ = cpu_->stack_[cpu_->sp_];
            if(cpu_->sp_ != 0u) {
                cpu_->sp_--;
            }
            break;
        }
        default:
        {
            // call 1802 program at 0XNNN
            std::print("opcode: {:#06x}. 0NNN is not implemented, reading the next instruction\n", cpu_->opcode);
            cpu_->pc_ += 2u;
            break;
        }
    }
}

void Emulation::opcode1X() {
    // 0x1nnn: jump to nnn
    cpu_->pc_ = cpu_->opcode & 0x0FFF;
}

void Emulation::opcode2X() {
    // 0x2nnn: push current address to stack and jump to nnn
    cpu_->stack_[cpu_->sp_] = cpu_->pc_;
    cpu_->sp_++;
    cpu_->pc_ = cpu_->opcode & 0x0FFF;
}

void Emulation::opcode3X() {
    // 0x3xnn: skip the next opcode if Vx == nn
    auto x = (cpu_->opcode >> 8) & 0x000F;
    auto value = cpu_->opcode & 0x00FF;
    cpu_->pc_ += (cpu_->V[x] == value ? 4u : 2u);
}

void Emulation::opcode4X() {
    // 0x4xnn: skip the next opcode if Vx != n
    auto x = (cpu_->opcode >> 8) & 0x000F;
    auto value = cpu_->opcode & 0x00FF;
    cpu_->pc_ += (cpu_->V[x] == value ? 2u : 4u);
}

void Emulation::opcode5X() {
    // 0x5xy0: skip next opcode if vX == vY
    auto x = (cpu_->opcode >> 8) & 0x000F;
    auto y = (cpu_->opcode >> 4) & 0x000F;
    cpu_->pc_ += (cpu_->V[x] == cpu_->V[y] ? 2u : 4u);
}

void Emulation::opcode6X() {
    // 0x6xnn: set Vx to nn
    auto x = (cpu_->opcode >> 8) & 0x000F;
    auto value = cpu_->opcode & 0x00FF;
    cpu_->V[x] = value;
    cpu_->pc_ += 2u;
}

void Emulation::opcode7X() {
    // 0x7xnn: add nn to Vx
    auto x = (cpu_->opcode >> 8) & 0x000F;
    auto value = cpu_->opcode & 0x00FF;
    cpu_->V[x] += value;
    cpu_->pc_ += 2u;
}

void Emulation::opcode8X() {
    auto low = cpu_->opcode & 0x000F;
    auto x = (cpu_->opcode >> 8) & 0x000F;
    auto y = (cpu_->opcode >> 4) & 0x000F;
    switch(low) {
        case 0u:
        {
            // 0x8xy0: set Vx to the value of Vy
            cpu_->V[x] = cpu_->V[y];
            break;
        }
        case 1u:
        {
            // 0x8xy1: set vX to the result of bitwise vX OR vY
            cpu_->V[x] |= cpu_->V[y];
            break;
        }
        case 2u:
        {
            // 0x8xy2: set vX to the result of bitwise vX AND vY
            cpu_->V[x] &= cpu_->V[y];
            break;
        }
        case 3u:
        {
            // 0x8xy3: set vX to the result of bitwise vX XOR vY
            cpu_->V[x] ^= cpu_->V[y];
            break;
        }
        case 4u:
        {
            // 0x8xy4: add vY to vX, vF is set to 1 if an overflow happened, to 0 if not, even if X=F!
            if(static_cast<uint32_t>(cpu_->V[x]) + static_cast<uint32_t>(cpu_->V[y]) > std::numeric_limits<std::uint8_t>::max()) {
                cpu_->V[0xF] = 1;
            } else {
                cpu_->V[0xF] = 0;
            }
            cpu_->V[x] += cpu_->V[y];
            break;
        }
        case 5u:
        {
            // 0x8xy5: subtract vY from vX, vF is set to 0 if an underflow happened, to 1 if not, even if X=F!
            if(cpu_->V[x] - cpu_->V[y] < 0) {
                cpu_->V[0xF] = 0u;
            } else {
                cpu_->V[0xF] = 1u;
            }
            cpu_->V[x] -= cpu_->V[y];
            break;
        }
        case 6u:
        {
            // 0x8xy6: set vX to vY and shift vX one bit to the right, set vF to the bit shifted out, even if X=F
            cpu_->V[0xF] = cpu_->V[x] & 0x1;
            cpu_->V[x] = cpu_->V[y] >> 1;
            break;
        }
        case 7u:
        {
            // 0x8xy7: set vX to the result of subtracting vX from vY, vF is set to 0 if an underflow happened, to 1 if not, even if X=F
            cpu_->V[0xF] = cpu_->V[x] > cpu_->V[y] ? 0u : 1u;
            cpu_->V[x] = cpu_->V[y] - cpu_->V[x];
            break;
        }
        case 0xE:
        {
            // 0x8xyE: set vX to vY and shift vX one bit to the left, set vF to the bit shifted out, even if X=F!
            cpu_->V[0xF] = (cpu_->V[x] >> 7) & 0x1;
            cpu_->V[x] = cpu_->V[x] << 1;
            break;
        }
        default:
        {
            std::print("Error: opcode {:#06x} is unknown\n", cpu_->opcode);
        }
    }
    cpu_->pc_ += 2u;
}

void Emulation::opcode9X() {
    // skip next opcode if vX != vY
    auto low = cpu_->opcode & 0x000F;
    auto x = (cpu_->opcode >> 8) & 0x000F;
    auto y = (cpu_->opcode >> 4) & 0x000F;

    cpu_->pc_ += (cpu_->V[x] == cpu_->V[y] ? 2u : 4u);
}

void Emulation::opcodeAX() {
    // 0xANNN: set I to NNN
    auto nnn = cpu_->opcode & 0x0FFF;
    cpu_->I = nnn;
    cpu_->pc_ += 2u;
}

void Emulation::opcodeBX() {
    // jump to address NNN + v0
    auto nnn = cpu_->opcode & 0x0FFF;
    cpu_->pc_ = nnn + cpu_->V[0];
}

void Emulation::opcodeCX() {
    auto nn = cpu_->opcode & 0x00FF;
    auto x = (cpu_->opcode >> 8) & 0x000F;
    std::mt19937 gen(rd_());
    std::uniform_int_distribution<> distrib(0u, 255u);
    cpu_->V[x] = distrib(gen) & nn;
    cpu_->pc_ += 2u;
}

void Emulation::opcodeDX() {
    // Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
    // Each row of 8 pixels is read as bit-coded starting from memory location I;
    // I value does not change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped
    // from set to unset when the sprite is drawn, and to 0 if that does not happen.
    std::print("Drawing not implemented yet...\n");
    cpu_->pc_ += 2u;
}

void Emulation::opcodeEX() {
    auto x = (cpu_->opcode >> 8) & 0x000F;
    auto nn =cpu_->opcode & 0x00FF;
    switch(nn) {
        case 0x9E:
        {
            // Skips the next instruction if the key stored in VX(only consider the lowest nibble) is pressed
            std::print("Key press event is not implemented...\n");
            cpu_->pc_ += 2u;
            break;
        }
        case 0xA1:
        {
            // Skip next opcode if key in the lower 4 bits of vX is not pressed
            std::print("Key press event is not implemented...\n");
            cpu_->pc_ += 4u;
            break;
        }
        default:
        {
            std::print("Error: opcode {:#06x} is unknown\n", cpu_->opcode);
        }
    }
}

void Emulation::opcodeFX() {
    auto x = (cpu_->opcode >> 8) & 0x000F;
    auto nn =cpu_->opcode & 0x00FF;
    switch(nn) {
        case 0x07:
        {
            // set Vx to the value of the delay timer
            cpu_->V[x] = cpu_->delay_timer_;
            break;
        }
        case 0x0A:
        {
            // wait for a key pressed and released and set vX to it, in megachip mode it also updates the screen like clear
            std::print("Key press event is not implemented...\n");
            break;
        }
        case 0x15:
        {
            // set delay timer to vX
            cpu_->delay_timer_ = cpu_->V[x];
            break;
        }
        case 0x18:
        {
            // set sound timer to vX, sound is played as long as the sound timer reaches zero
            cpu_->sound_timer_ = cpu_->V[x];
            break;
        }
        case 0x1E:
        {
            // add Vx to I
            cpu_->I += cpu_->V[x];
            break;
        }
        case 0x29:
        {
            // set I to the 5 line high hex sprite for the lowest nibble in vX
            std::print("Drawing not implemented yet...\n");
            break;
        }
        case 0x33:
        {
            // write the value of vX as BCD value at the addresses I, I+1 and I+2
            cpu_->memory_[cpu_->I] = std::byte{static_cast<uint8_t>((cpu_->V[x] % 1000) / 100)};
            cpu_->memory_[cpu_->I + 1] = std::byte{static_cast<uint8_t>((cpu_->V[x] % 100) / 10)};
            cpu_->memory_[cpu_->I + 2] = std::byte{static_cast<uint8_t>(cpu_->V[x] % 10)};
            break;
        }
        case 0x55:
        {
            // write the content of v0 to vX at the memory pointed to by I, I is incremented by X+1
            for(auto i = 0; i < 15; ++i) {
                cpu_->memory_[cpu_->I + i] = std::byte{cpu_->V[i]};
            }
            break;
        }
        case 0x65:
        {
            // read the bytes from memory pointed to by I into the registers v0 to vX, I is incremented by X+1
            for(auto i = 0; i < 15; ++i) {
                cpu_->V[i] = std::to_integer<std::uint8_t>(cpu_->memory_[cpu_->I + i]);
            }
            break;
        }
        default:
        {
            std::print("Error: opcode {:#06x} is unknown\n", cpu_->opcode);
        }
    }
    cpu_->pc_ += 2u;
}
} // namespace chippit