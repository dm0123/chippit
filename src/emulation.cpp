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
Emulation::Emulation(bool cpuDump, bool memoryDump, bool graphicsDump)
    : cpu_{std::make_unique<Chip8>()}, graphics_{cpu_.get()}, app_{*this, graphics_, input_}, rom_{cpu_.get()},
       cpuDump_{cpuDump}, memoryDump_{memoryDump}, graphicsDump_{graphicsDump} {
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

    for(auto i = 0u; i < 80u; ++i) {
        cpu_->memory_[i] = Chip8::fontset_[i];
    }
}

void Emulation::run() {
    std::print("Emulation::run()\n");

    cpuThreadFuture_ = std::async(std::launch::async, std::bind(&Emulation::cpu_thread, this));
    app_.run();
}

void Emulation::stop() {
    finished_.store(true);
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
        cpu_tick();
        std::this_thread::sleep_for(frequency);
    }
    reset();
}

void Emulation::cpu_tick() {
    std::unique_lock<std::mutex> lock_{cpuMutex_};
    fetch();
    decodeAndExecute();
    updateTimers();
}

void Emulation::dump() {
    if(cpuDump_) {
        std::print("CPU State: \n");
        std::print("I: {} ", cpu_->I);
        for(int i = 0; i < 16; ++i) {
            std::print("V[{}]: {} ", i, cpu_->V[i]);
        }

        std::print("\npc: {}, sp: {}", cpu_->pc_, cpu_->sp_);
        std::print("\n ===== stack dump =====");
        for(auto byte = 0u; byte < 16; ++byte){
            std::print("{:#06x} ", cpu_->stack_[byte]);
        }
        std::print("\n\n");
    }

    auto i = 0;
    if(memoryDump_) {
        for(int row = 0; row < 256; ++row) {
            for(int column = 0; column < 16; ++column) {
                std::print("{:#06x} ", cpu_->memory_[i]);
                ++i;
            }
            std::print("\n");
        }

        std::print("\n\n");
    }

    i = 0;

    if(graphicsDump_) {
        for(int row = 0; row < 32; ++row) {
            for(int column = 0; column < 64; ++column) {
                std::print("{}", (cpu_->graphics_[i] == 0 ? "." : "*"));
                ++i;
            }
            std::print("\n");
        }
        std::print("\n\n");
    }
}

void Emulation::fetch() {
    cpu_->opcode = cpu_->memory_[cpu_->pc_] << 8 | cpu_->memory_[cpu_->pc_ + 1]; // Big-endian
}

void Emulation::decodeAndExecute() {
    std::uint16_t high = (cpu_->opcode >> 8) >> 4;
    if(high > opcodeTable_.size()) {
        std::print("Unknown opcode {:#06x}\n", cpu_->opcode);
        cpu_->pc_ += 2;
        return;
    }
    auto& func = opcodeTable_.at(high);
    if(cpuDump_ || memoryDump_ || graphicsDump_) {
        std::print("Executing  opcode {:#06x}\n", cpu_->opcode);
    }
    func();
    dump();
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
            std::fill(std::begin(cpu_->graphics_), std::end(cpu_->graphics_), 0u);
            cpu_->pc_ += 2u;
            break;
        }
        case 0x00EE:
        {
            // return to address pulled from stack
            --cpu_->sp_;
            cpu_->pc_ = cpu_->stack_[cpu_->sp_];
            cpu_->pc_ += 2u;
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
    ++cpu_->sp_;
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
    cpu_->pc_ += (cpu_->V[x] != value ? 4u : 2u);
}

void Emulation::opcode5X() {
    // 0x5xy0: skip next opcode if vX == vY
    auto x = (cpu_->opcode >> 8) & 0x000F;
    auto y = (cpu_->opcode >> 4) & 0x000F;
    cpu_->pc_ += (cpu_->V[x] == cpu_->V[y] ? 4u : 2u);
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
            if(cpu_->V[x] < cpu_->V[y]) {
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
            cpu_->V[0xF] = cpu_->V[x] >> 7;
            cpu_->V[x] <<= 1;
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
    auto x = cpu_->V[(cpu_->opcode >> 8) & 0x000F];
    auto y = cpu_->V[(cpu_->opcode >> 4) & 0x000F];
    auto n = cpu_->opcode & 0x000F;
    std::uint8_t pixel = 0u;

    cpu_->V[0xF] = 0u;
    for(auto line_y = 0u; line_y < n; ++line_y) {
        pixel = cpu_->memory_[cpu_->I + line_y];
        for(auto line_x = 0u; line_x < 8u; ++line_x) {
            if((pixel & (0x80 >> line_x)) != 0u) {
                auto mem_location = x + line_x + ((y + line_y) * 64u);
                if(cpu_->graphics_[mem_location] == 1) {
                    cpu_->V[0xF] = 1u;
                }

                cpu_->graphics_[mem_location] ^= 1;
            }
        }
    }
    cpu_->pc_ += 2u;
}

void Emulation::opcodeEX() {
    auto x = (cpu_->opcode >> 8) & 0x000F;
    auto nn = cpu_->opcode & 0x00FF;
    switch(nn) {
        case 0x9E:
        {
            // Skips the next instruction if the key stored in VX(only consider the lowest nibble) is pressed
            auto keycode = cpu_->V[x] & 0x00FF;
            if(keycode > 16u) {
                std::print("Unknown keycode: {}\n", keycode);
                cpu_->pc_ += 2u;
                break;
            }
            if(input_.isPressed(static_cast<Input::Key>(keycode))) {
                cpu_->pc_ += 4u;
                break;
            }
            cpu_->pc_ += 2u;
            break;
        }
        case 0xA1:
        {
            // Skip next opcode if key in the lower 4 bits of vX is not pressed
            auto keycode = cpu_->V[x] & 0x00FF;
            if(keycode > 16u) {
                std::print("Unknown keycode: {}\n", keycode);
                cpu_->pc_ += 2u;
                break;
            }
            if(!input_.isPressed(static_cast<Input::Key>(keycode))) {
                cpu_->pc_ += 4u;
                break;
            }
            cpu_->pc_ += 2u;
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
            if(!input_.isPressed()) {
                return;
            }
            for(auto i = 0; i < 16; ++i) {
                if(input_.isPressed(static_cast<Input::Key>(i))) {
                    cpu_->V[x] = i;
                }
            }
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
            if(cpu_->I + cpu_->V[x] > 0xFFF) {
                cpu_->V[0xF] = 1;
            } else {
                cpu_->V[0xF] = 0;
            }
            cpu_->I += cpu_->V[x];
            break;
        }
        case 0x29:
        {
            // set I to the 5 line high hex sprite for the lowest nibble in vX
            cpu_->I = cpu_->V[x] * 0x5;
            break;
        }
        case 0x33:
        {
            // write the value of vX as BCD value at the addresses I, I+1 and I+2
            cpu_->memory_[cpu_->I] = cpu_->V[x] / 100;
            cpu_->memory_[cpu_->I + 1] = static_cast<uint8_t>((cpu_->V[x] / 10) % 10);
            cpu_->memory_[cpu_->I + 2] = static_cast<uint8_t>(cpu_->V[x] % 10);
            break;
        }
        case 0x55:
        {
            // write the content of v0 to vX at the memory pointed to by I, I is incremented by X+1
            for(auto i = 0; i < x; ++i) {
                cpu_->memory_[cpu_->I + i] = cpu_->V[i];
            }
            cpu_->I += x + 1;
            break;
        }
        case 0x65:
        {
            // read the bytes from memory pointed to by I into the registers v0 to vX, I is incremented by X+1
            for(auto i = 0; i < x; ++i) {
                cpu_->V[i] = cpu_->memory_[cpu_->I + i];
            }
            cpu_->I += x + 1;
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