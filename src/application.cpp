module;

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

#include <iostream>
#include <format>
#include <future>
#include <functional>

module application;
import emulation;

namespace chippit {
Application::Application(Emulation& emu, Graphics& graphics, Input& input) 
    : emu_{emu}, graphics_{graphics}, input_{input} {

}

void Application::run() {
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cout << std::format("Error initializing SDL: {}\n", SDL_GetError());
        return;
    }
    SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_SHOWN, &window_, &renderer_);
    if(!window_) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << "\n"; // TODO: logger
        return;
    }

    SDL_SetWindowTitle(window_, "CHIP8");
    graphics_.init(window_, renderer_);
    while(!finished_.load()) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if(e.type == SDL_KEYDOWN) {
                input_.setPressed(e.key.keysym.sym, true);
            }
            else if(e.type == SDL_KEYUP) {
                input_.setPressed(e.key.keysym.sym, false);
            }
            else if (e.type == SDL_QUIT) {
                finished_.store(true);
            }
        }
        graphics_.update();
    }

    emu_.stop();

    graphics_.deinit();

    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}
} // namespace chippit