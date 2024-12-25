module;
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

#include <iostream>
#include <format>
#include <future>
#include <functional>

export module graphics;
import core;

namespace chippit {
export class Graphics {
public:
    Graphics(Chip8* cpu) : cpu_{cpu} {
    }

    ~Graphics() {
        deinit();
    }

    bool update(SDL_Window* window, SDL_Renderer* renderer) {
        // This method is called from the main thread
        if(!window || !renderer || !cpu_) {
            // TODO: logger
            return false;
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for(auto i = 0u; i < sizeof(cpu_->graphics_); ++i) {
            SDL_RenderDrawPoint(renderer, i, i);
        }
        SDL_RenderPresent(renderer);

        return true;
    }

    bool init() {
    
        return true;
    }

    void deinit() {
        
    }

private:
    Chip8* cpu_{nullptr};
};
} // namespace chippit