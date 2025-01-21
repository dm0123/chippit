module;
#include <cstdint>
#include <cstddef>
#include <variant>

export module core;

namespace chippit {
namespace constants {
export constexpr size_t display_width = 64u;
export constexpr size_t display_height = 32u;
} // namespace constants

export struct Chip8 {
    std::uint8_t V[16];
    std::uint16_t opcode;
    std::uint16_t I;
    std::uint16_t pc_;
    std::uint8_t stack_[16];
    std::uint8_t sp_;

    // TODO: may be separate classes, but there isn't much code
    std::byte memory_[4096]; // TODO: std::byte always seems to need convertation...
    std::byte graphics_[constants::display_width * constants::display_height];

    std::uint8_t delay_timer_;
    std::uint8_t sound_timer_;

    // input
    std::uint8_t key_[16];

    static constexpr std::byte fontset_[80] = {
        std::byte{0xF0}, std::byte{0x90}, std::byte{0x90}, std::byte{0x90}, std::byte{0xF0}, // 0
        std::byte{0x20}, std::byte{0x60}, std::byte{0x20}, std::byte{0x20}, std::byte{0x70}, // 1
        std::byte{0xF0}, std::byte{0x10}, std::byte{0xF0}, std::byte{0x80}, std::byte{0xF0}, // 2
        std::byte{0xF0}, std::byte{0x10}, std::byte{0xF0}, std::byte{0x10}, std::byte{0xF0}, // 3
        std::byte{0x90}, std::byte{0x90}, std::byte{0xF0}, std::byte{0x10}, std::byte{0x10}, // 4
        std::byte{0xF0}, std::byte{0x80}, std::byte{0xF0}, std::byte{0x10}, std::byte{0xF0}, // 5
        std::byte{0xF0}, std::byte{0x80}, std::byte{0xF0}, std::byte{0x90}, std::byte{0xF0}, // 6
        std::byte{0xF0}, std::byte{0x10}, std::byte{0x20}, std::byte{0x40}, std::byte{0x40}, // 7
        std::byte{0xF0}, std::byte{0x90}, std::byte{0xF0}, std::byte{0x90}, std::byte{0xF0}, // 8
        std::byte{0xF0}, std::byte{0x90}, std::byte{0xF0}, std::byte{0x10}, std::byte{0xF0}, // 9
        std::byte{0xF0}, std::byte{0x90}, std::byte{0xF0}, std::byte{0x90}, std::byte{0x90}, // A
        std::byte{0xE0}, std::byte{0x90}, std::byte{0xE0}, std::byte{0x90}, std::byte{0xE0}, // B
        std::byte{0xF0}, std::byte{0x80}, std::byte{0x80}, std::byte{0x80}, std::byte{0xF0}, // C
        std::byte{0xE0}, std::byte{0x90}, std::byte{0x90}, std::byte{0x90}, std::byte{0xE0}, // D
        std::byte{0xF0}, std::byte{0x80}, std::byte{0xF0}, std::byte{0x80}, std::byte{0xF0}, // E
        std::byte{0xF0}, std::byte{0x80}, std::byte{0xF0}, std::byte{0x80}, std::byte{0x80}  // F
    };
};
} // namespace chippit