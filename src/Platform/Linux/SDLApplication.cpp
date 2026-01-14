// src/SDLApplication.cpp
#include "Platform/Linux/SDLApplication.hpp"
#include <SDL.h>
#include <iostream>

SDLApplication::SDLApplication(std::string caption, int width, int height)
    : _caption(std::move(caption)),
      _width(static_cast<uint32_t>(width)),
      _height(static_cast<uint32_t>(height)),
      _running(false) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Izotrox: SDL_Init failed: " << SDL_GetError() << std::endl;
        return;
    }

    window_ = SDL_CreateWindow(_caption.c_str(),
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

    _running = true;
}

SDLApplication::~SDLApplication() {
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_)   SDL_DestroyWindow(window_);
    SDL_Quit();
}

void SDLApplication::run() {
    while (_running) {
        process_events();
        update();
        draw();
    }
}

bool SDLApplication::is_running() { return _running; }
uint32_t SDLApplication::width()   { return _width; }
uint32_t SDLApplication::height()  { return _height; }
std::string SDLApplication::caption() { return _caption; }

void SDLApplication::process_events() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            _running = false;
        }
    }
}

void SDLApplication::update() {
    // Application state updates go here
}

void SDLApplication::draw() {
    SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 255);
    SDL_RenderClear(renderer_);

    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_Rect rect{
        static_cast<int>(_width / 2 - 100),
        static_cast<int>(_height / 2 - 75),
        200,
        150};
    SDL_RenderFillRect(renderer_, &rect);

    SDL_RenderPresent(renderer_);
}
