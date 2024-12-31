module;
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

#include <cstddef>
#include <iostream>
#include <format>
#include <future>
#include <functional>

export module graphics;
import core;

namespace chippit {
/// @brief Graphical representation of the emulated system
/// @details Graphics class, currently assumes SDL2 software rendering only
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
        auto screen_x = 0u;
        auto screen_y = 0u;
        for(auto i = 0u; i < constants::display_width * constants::display_height; ++i) {
                screen_x++;
                if(screen_x == constants::display_width) {
                    screen_x = 0;
                    screen_y++;
                }

                auto color = (cpu_->graphics_[i] > std::byte{0}) ? 255 : 0;
                SDL_SetRenderDrawColor(renderer, color, color, color, 255); // TODO: maybe some Color class, but it's black-white...
                SDL_RenderDrawPoint(renderer, screen_x, screen_y); // TODO: upscaling goes there
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