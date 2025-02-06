module;
#include <bitset>
#include <cstdint>
#include <map>
#include <mutex>

#include <SDL2/SDL.h>

export module input;

namespace chippit {
export class Input {
public:
    enum class Key : std::uint16_t {
        KEY0 = 0u, KEY1, KEY2, KEY3, KEY4, KEY5, KEY6, KEY7, KEY8, KEY9,
        KEYA, KEYB, KEYC, KEYD, KEYE, KEYF,
        KEYR, KEYESC, KEYQ, KEYP
    };

    bool isPressed(Key key) {
        return pressedKeys_[static_cast<std::size_t>(key)];
    }

    bool isPressed() {
        return pressedKeys_.any();
    }

    void setPressed(Key key, bool pressed) {
        std::unique_lock<std::mutex> lock{inputMutex_};
        pressedKeys_.set(static_cast<size_t>(key), pressed);
    }

    void setPressed(SDL_Keycode sdlKey, bool pressed) {
        if(!conversion_.contains(sdlKey)) {
            return;
        }
        setPressed(conversion_.at(sdlKey), pressed);
    }

private:
    const std::map<SDL_Keycode, Key> conversion_ {
        {SDLK_0, Key::KEY0},
        {SDLK_1, Key::KEY1},
        {SDLK_2, Key::KEY2},
        {SDLK_3, Key::KEY3},
        {SDLK_4, Key::KEY4},
        {SDLK_5, Key::KEY5},
        {SDLK_6, Key::KEY6},
        {SDLK_7, Key::KEY7},
        {SDLK_8, Key::KEY8},
        {SDLK_9, Key::KEY9},
        {SDLK_a, Key::KEYA},
        {SDLK_b, Key::KEYB},
        {SDLK_c, Key::KEYC},
        {SDLK_d, Key::KEYD},
        {SDLK_e, Key::KEYE},
        {SDLK_f, Key::KEYF},
    };

    std::bitset<20> pressedKeys_{};
    std::mutex inputMutex_;
};
} // namespace chippit