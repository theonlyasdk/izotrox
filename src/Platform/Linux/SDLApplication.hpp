

#pragma once

#include <SDL.h>
#include <cstdint>
#include <string>
#include <functional>

class SDLApplication {
public:
    SDLApplication(std::string caption, int width = 800, int height = 600);
    ~SDLApplication();

    bool pump_events();
    void present(const uint32_t* pixels, int width, int height);
    void quit(int exit_code);

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    void set_on_resize(std::function<void(int, int)> cb) { m_on_resize = cb; }

private:
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture*  m_texture  = nullptr;
    bool          m_running  = false;
    uint32_t      m_width, m_height;

    std::function<void(int, int)> m_on_resize;
};
