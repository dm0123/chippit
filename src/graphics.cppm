module;
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

#include <cstddef>
#include <cstring>
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

    void init(SDL_Window* window, SDL_Renderer* renderer) {
        window_ = window;
        renderer_ = renderer;
    }

    void deinit() {
    }

    bool update() {
        if(!window_ || !renderer_) {
            std::print("FATAL: no window\n");
            return false;
        }

        auto rec_w = 0;
        auto rec_h = 0;

        SDL_GetWindowSize(window_, &rec_w, &rec_h);
        rec_w /= constants::display_width;
        rec_h /= constants::display_height;
        SDL_RenderClear(renderer_);

        for(int row = 0; row < constants::display_width; ++row) {
            for(int column = 0; column < constants::display_height; ++column) {
                auto graphics_pos = row + column * constants::display_width;
                auto color = cpu_->graphics_[graphics_pos] == 1u ? 255u : 0u;
                SDL_Rect rect;
                rect.x = row * rec_w;
                rect.y = column * rec_h;
                rect.w = rec_w;
                rect.h = rec_h;
                SDL_SetRenderDrawColor(renderer_, color, color, color, 255);
                SDL_RenderFillRect(renderer_, &rect);
                SDL_RenderDrawRect(renderer_, &rect);

                SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            }
        }

        SDL_RenderPresent(renderer_);

        return true;
    }
private:
    Chip8* cpu_{nullptr};
    SDL_Window* window_{nullptr};
    SDL_Renderer* renderer_{nullptr};
};
} // namespace chippit