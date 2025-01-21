module;
#include <string_view>
#include <fstream>

export module rom;
import core;

namespace chippit {
// TODO: this is a stateless object, but the name suggests otherwise. Bad design?
export class Rom {
public:
    static constexpr std::streamsize max_rom_size = 0x1000 - 0x200;

    Rom(Chip8* cpu) : cpu_{cpu} {
    }

    bool load(std::string_view romPath) {
        if(!cpu_) {
            std::print("Fatal: CPU is nullptr\n");
            return false;
        }
        std::ifstream rom{romPath, std::ios::binary};
        if(!rom.is_open()) {
            std::print("Failed to open '{}'", romPath);
            return false;
        }

        // TODO: this is a bit blunt and handles errors poorly
        rom.read(reinterpret_cast<char*>(&cpu_->memory_[0x200]), max_rom_size);
        return true;
    }
private:
    Chip8* cpu_{nullptr};
};
} // namespace chippit