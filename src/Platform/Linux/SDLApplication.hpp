#pragma once

#include <SDL.h>
#include <cstdint>
#include <string>

class SDLApplication {
public:
    SDLApplication(std::string caption, int width = 800, int height = 600);
    ~SDLApplication();

    // Returns false if the application should quit
    bool pump_events();
    
    // Blit raw pixels to the window
    void present(const uint32_t* pixels, int width, int height);

    uint32_t width() const { return _width; }
    uint32_t height() const { return _height; }

private:
    SDL_Window*   window_   = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture*  texture_  = nullptr;
    bool          _running  = false;
    uint32_t      _width, _height;
};