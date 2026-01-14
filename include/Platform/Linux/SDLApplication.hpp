// include/SDLApplication.hpp
#pragma once

#include <SDL.h>
#include <cstdint>
#include <string>

class SDLApplication {
public:
    SDLApplication(std::string caption, int width = 800, int height = 600);
    ~SDLApplication();
    void run();

    bool is_running();
    uint32_t width();
    uint32_t height();
    std::string caption();
private:
    void process_events();
    void update();
    void draw();

    SDL_Window*   window_   = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    bool          _running  = false;
    std::string _caption;
    uint32_t _width, _height;
};
