module;
#include <cstdint>
#include <cstddef>

export module core;

namespace chippit {
namespace constants {
export constexpr size_t display_width = 64u;
export constexpr size_t display_height = 32u;
} // namespace constants

export struct Chip8 {
    struct Registers {
        std::uint8_t V0;
        std::uint8_t V1;
        std::uint8_t V2;
        std::uint8_t V3;
        std::uint8_t V4;
        std::uint8_t V5;
        std::uint8_t V6;
        std::uint8_t V7;
        std::uint8_t V8;
        std::uint8_t V9;
        std::uint8_t VA;
        std::uint8_t VB;
        std::uint8_t VC;
        std::uint8_t VD;
        std::uint8_t VE;
        std::uint8_t VF;
        
    };

    Registers registers;

    std::uint16_t opcode;
    std::uint16_t I;
    std::uint16_t pc_;
    std::uint8_t stack_[16];
    std::uint8_t sp_;

    // TODO: may be separate classes, but there isn't much code
    std::byte memory_[4096];
    std::byte graphics_[constants::display_width * constants::display_height];

    std::uint8_t delay_timer_;
    std::uint8_t sound_timer_;

    // input
    std::uint8_t key_[16];
};
} // namespace chippit