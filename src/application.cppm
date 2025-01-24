module;
#include <atomic>

#include <SDL2/SDL.h>

export module application;

import graphics;
import input;

namespace chippit {
class Emulation;

/// @brief Main application class
/// @details The purpose of this class is to hold the main loop
///          and everything that is needed to launch it from
///          the OS perspective
export class Application final {
public:
    Application(Emulation& emu, Graphics& graphics, Input& input);
    void run();

    ~Application() = default;
private:
    Graphics& graphics_;
    Input& input_;
    Emulation& emu_;
    std::atomic_bool finished_{false};

    SDL_Window* window_{nullptr};
    SDL_Renderer* renderer_{nullptr};
    SDL_Event event_;
};
}