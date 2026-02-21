#include "Platform/Linux/SDLApplication.hpp"
#include <SDL.h>
#include <SDL_render.h>
#include "Debug/Logger.hpp"
#include "Geometry/Primitives.hpp"
#include "Graphics/Color.hpp"
#include "Input/Input.hpp"

using namespace Izo;

SDLApplication::SDLApplication(std::string caption, int width, int height)
    : m_width(static_cast<uint32_t>(width)),
      m_height(static_cast<uint32_t>(height)),
      m_running(false) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LogError("SDL_Init failed: {}", SDL_GetError());
        return;
    }

    SDL_StartTextInput();

    m_window = SDL_CreateWindow(caption.c_str(),
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               static_cast<int>(m_width),
                               static_cast<int>(m_height),
                               SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
    if (!m_window) {
        LogError("SDL_CreateWindow failed: {}", SDL_GetError());
        SDL_Quit();
        return;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1,
                                   SDL_RENDERER_ACCELERATED |
                                   SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) {
        LogError("SDL_CreateRenderer failed: {}", SDL_GetError());
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return;
    }

    // Log renderer information
    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(m_renderer, &info) == 0) {
        LogInfo("SDL Renderer initialized: {}", info.name);
        if (info.flags & SDL_RENDERER_ACCELERATED) {
            LogInfo("Hardware acceleration enabled");
        } else {
            LogWarn("Hardware acceleration not available, using software rendering");
        }
        if (info.flags & SDL_RENDERER_PRESENTVSYNC) {
            LogInfo("VSync enabled");
        }
    }

    m_texture = SDL_CreateTexture(m_renderer,
                                 SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 m_width, m_height);
    if (!m_texture) {
        LogError("SDL_CreateTexture failed: {}", SDL_GetError());
    }

    m_running = true;
}

SDLApplication::~SDLApplication() {
    if (m_texture)  SDL_DestroyTexture(m_texture);
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window)   SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool SDLApplication::pump_events() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            m_running = false;
        } else if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                int w = e.window.data1;
                int h = e.window.data2;
                if (w <= 0 || h <= 0) continue;

                m_width = w;
                m_height = h;

                if (m_texture) SDL_DestroyTexture(m_texture);
                m_texture = SDL_CreateTexture(m_renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             m_width, m_height);

                if (m_on_resize) m_on_resize(m_width, m_height);
            }
        } else if (e.type == SDL_MOUSEWHEEL) {
             Izo::Input::the().set_scroll(e.wheel.y);
        } else if (e.type == SDL_MOUSEMOTION) {
            Izo::Input::the().set_touch({e.motion.x, e.motion.y}, (e.motion.state & SDL_BUTTON_LMASK) != 0);
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
             Izo::Input::the().set_touch({e.button.x, e.button.y}, true);
        } else if (e.type == SDL_MOUSEBUTTONUP) {
             Izo::Input::the().set_touch({e.button.x, e.button.y}, false);
        } else if (e.type == SDL_TEXTINPUT) {
            if (e.text.text[0]) {
                 Izo::Input::the().set_key((Izo::KeyCode)e.text.text[0]);
            }
        } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
             bool down = (e.type == SDL_KEYDOWN);
             if (e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT) {
                 Izo::Input::the().set_shift(down);
             } else if (e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
                 Izo::Input::the().set_ctrl(down);
             }

             if (down) {
                 if (e.key.keysym.sym == SDLK_BACKSPACE) {
                     Izo::Input::the().set_key(Izo::KeyCode::Backspace);
                 } else if (e.key.keysym.sym == SDLK_DELETE) {
                     Izo::Input::the().set_key(Izo::KeyCode::Delete);
                 } else if (e.key.keysym.sym == SDLK_RETURN) {
                     Izo::Input::the().set_key(Izo::KeyCode::Enter);
                 } else if (e.key.keysym.sym == SDLK_LEFT) {
                     Izo::Input::the().set_key(Izo::KeyCode::Left);
                 } else if (e.key.keysym.sym == SDLK_RIGHT) {
                     Izo::Input::the().set_key(Izo::KeyCode::Right);
                 } else if (e.key.keysym.sym == SDLK_UP) {
                     Izo::Input::the().set_key(Izo::KeyCode::Up);
                 } else if (e.key.keysym.sym == SDLK_DOWN) {
                     Izo::Input::the().set_key(Izo::KeyCode::Down);
                 } else if (e.key.keysym.sym == SDLK_HOME) {
                     Izo::Input::the().set_key(Izo::KeyCode::Home);
                 } else if (e.key.keysym.sym == SDLK_END) {
                     Izo::Input::the().set_key(Izo::KeyCode::End);
                 } else if (e.key.keysym.sym == SDLK_PAGEUP) {
                     Izo::Input::the().set_key(Izo::KeyCode::PageUp);
                 } else if (e.key.keysym.sym == SDLK_PAGEDOWN) {
                     Izo::Input::the().set_key(Izo::KeyCode::PageDown);
                 } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                     Izo::Input::the().set_key(Izo::KeyCode::Escape);
                 } else if (e.key.keysym.sym == SDLK_a && (e.key.keysym.mod & KMOD_CTRL)) {
                     Izo::Input::the().set_key((Izo::KeyCode)'a');
                 } else if (e.key.keysym.sym == SDLK_c && (e.key.keysym.mod & KMOD_CTRL)) {
                     Izo::Input::the().set_key((Izo::KeyCode)'c');
                 } else if (e.key.keysym.sym == SDLK_v && (e.key.keysym.mod & KMOD_CTRL)) {
                     Izo::Input::the().set_key((Izo::KeyCode)'v');
                 } else if (e.key.keysym.sym == SDLK_x && (e.key.keysym.mod & KMOD_CTRL)) {
                     Izo::Input::the().set_key((Izo::KeyCode)'x');
                 }
             }
        }
    }
    return m_running;
}

void SDLApplication::draw_pixel(const IntPoint& point, Color color)
{
    // Set the drawing colour for the renderer.
    // SDL expects the components in the order RGBA, each as an 8‑bit value.
    SDL_SetRenderDrawColor(
        m_renderer,
        static_cast<uint8_t>(color.r),   // red
        static_cast<uint8_t>(color.g),   // green
        static_cast<uint8_t>(color.b),   // blue
        static_cast<uint8_t>(color.a)    // alpha (0 = transparent, 255 = opaque)
    );

    // Actually draw the pixel at the requested coordinates.
    SDL_RenderDrawPoint(m_renderer, point.x, point.y);
}


void SDLApplication::present(const uint32_t* pixels, int width, int height) {
    if (!m_texture || !m_renderer || !pixels) return;

    if (width != (int)m_width || height != (int)m_height) {
        return;
    }

    SDL_UpdateTexture(m_texture, NULL, pixels, width * sizeof(uint32_t));

    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
    SDL_RenderPresent(m_renderer);
}

void SDLApplication::quit(int exit_code) {
    // Exit immediately on failure
    if (exit_code < 0)
        exit(exit_code);

    m_running = false;
}

void SDLApplication::show() {
    if (m_window) {
        SDL_ShowWindow(m_window);
    }
}
