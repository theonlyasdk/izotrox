#include "Platform/Linux/SDLApplication.hpp"
#include <SDL.h>
#include <iostream>
#include "Input/Input.hpp"

SDLApplication::SDLApplication(std::string caption, int width, int height)
    : _width(static_cast<uint32_t>(width)),
      _height(static_cast<uint32_t>(height)),
      _running(false) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Izotrox: SDL_Init failed: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Enable text input for keyboard handling
    SDL_StartTextInput();

    window_ = SDL_CreateWindow(caption.c_str(),
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               static_cast<int>(_width),
                               static_cast<int>(_height),
                               SDL_WINDOW_SHOWN);
    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    renderer_ = SDL_CreateRenderer(window_, -1,
                                   SDL_RENDERER_ACCELERATED |
                                   SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window_);
        SDL_Quit();
        return;
    }

    texture_ = SDL_CreateTexture(renderer_,
                                 SDL_PIXELFORMAT_ARGB8888, // Matches our Color struct (mostly)
                                 SDL_TEXTUREACCESS_STREAMING,
                                 _width, _height);
    if (!texture_) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
    }

    _running = true;
}

SDLApplication::~SDLApplication() {
    if (texture_)  SDL_DestroyTexture(texture_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_)   SDL_DestroyWindow(window_);
    SDL_Quit();
}

bool SDLApplication::pump_events() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            _running = false;
        } else if (e.type == SDL_MOUSEMOTION) {
            Izo::Input::instance().set_touch(e.motion.x, e.motion.y, (e.motion.state & SDL_BUTTON_LMASK) != 0);
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
             Izo::Input::instance().set_touch(e.button.x, e.button.y, true);
        } else if (e.type == SDL_MOUSEBUTTONUP) {
             Izo::Input::instance().set_touch(e.button.x, e.button.y, false);
        } else if (e.type == SDL_TEXTINPUT) {
            // Simple handling for ASCII
            if (e.text.text[0]) {
                 Izo::Input::instance().set_key((int)e.text.text[0]);
            }
        } else if (e.type == SDL_KEYDOWN) {
             if (e.key.keysym.sym == SDLK_BACKSPACE) {
                 Izo::Input::instance().set_key(8); // Backspace ASCII
             } else if (e.key.keysym.sym == SDLK_RETURN) {
                 Izo::Input::instance().set_key(13); // Enter
             }
        }
    }
    return _running;
}

void SDLApplication::present(const uint32_t* pixels, int width, int height) {
    if (!texture_ || !renderer_) return;

    // We assume width/height matches or we just update what we can
    // In a real app we might handle resizing
    SDL_UpdateTexture(texture_, NULL, pixels, width * sizeof(uint32_t));
    
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, NULL, NULL);
    SDL_RenderPresent(renderer_);
}