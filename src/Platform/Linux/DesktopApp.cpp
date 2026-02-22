#include "Platform/Linux/DesktopApp.hpp"

#include <SDL_render.h>
#include <cstdlib>

#include "Debug/Logger.hpp"
#include "Graphics/Canvas.hpp"
#include "Input/Input.hpp"

namespace Izo {

DesktopApp::DesktopApp(std::string caption, int width, int height)
    : m_caption(std::move(caption)),
      m_width(static_cast<uint32_t>(width)),
      m_height(static_cast<uint32_t>(height)) {
}

DesktopApp::~DesktopApp() {
    if (m_texture) SDL_DestroyTexture(m_texture);
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window) SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool DesktopApp::recreate_texture(uint32_t w, uint32_t h) {
    if (!m_renderer) return false;

    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }

    m_texture = SDL_CreateTexture(
        m_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        static_cast<int>(w),
        static_cast<int>(h)
    );

    if (!m_texture) {
        LogError("SDL_CreateTexture failed: {}", SDL_GetError());
        return false;
    }
    m_texture_needs_full_upload = true;
    return true;
}

bool DesktopApp::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LogError("SDL_Init failed: {}", SDL_GetError());
        return false;
    }

    SDL_StartTextInput();

    m_window = SDL_CreateWindow(
        m_caption.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        static_cast<int>(m_width),
        static_cast<int>(m_height),
        SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE
    );
    if (!m_window) {
        LogError("SDL_CreateWindow failed: {}", SDL_GetError());
        return false;
    }

    m_renderer = SDL_CreateRenderer(
        m_window,
        -1,
        SDL_RENDERER_ACCELERATED
    );
    if (!m_renderer) {
        LogError("SDL_CreateRenderer failed: {}", SDL_GetError());
        return false;
    }

    SDL_RendererInfo info{};
    if (SDL_GetRendererInfo(m_renderer, &info) == 0) {
        LogInfo("SDL Renderer initialized: {}", info.name);
    }

    if (!recreate_texture(m_width, m_height)) {
        return false;
    }

    m_running = true;
    return true;
}

bool DesktopApp::pump_events() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            m_running = false;
        } else if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                int w = e.window.data1;
                int h = e.window.data2;
                if (w <= 0 || h <= 0) continue;

                m_width = static_cast<uint32_t>(w);
                m_height = static_cast<uint32_t>(h);

                recreate_texture(m_width, m_height);
                if (m_on_resize) m_on_resize(w, h);
            }
        } else if (e.type == SDL_MOUSEWHEEL) {
            Input::the().set_scroll(e.wheel.y);
        } else if (e.type == SDL_MOUSEMOTION) {
            Input::the().set_touch({e.motion.x, e.motion.y}, (e.motion.state & SDL_BUTTON_LMASK) != 0);
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            Input::the().set_touch({e.button.x, e.button.y}, true);
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            Input::the().set_touch({e.button.x, e.button.y}, false);
        } else if (e.type == SDL_TEXTINPUT) {
            if (e.text.text[0]) {
                Input::the().set_key((KeyCode)e.text.text[0]);
            }
        } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            bool down = (e.type == SDL_KEYDOWN);
            if (e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT) {
                Input::the().set_shift(down);
            } else if (e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
                Input::the().set_ctrl(down);
            }

            if (down) {
                if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    Input::the().set_key(KeyCode::Backspace);
                } else if (e.key.keysym.sym == SDLK_DELETE) {
                    Input::the().set_key(KeyCode::Delete);
                } else if (e.key.keysym.sym == SDLK_RETURN) {
                    Input::the().set_key(KeyCode::Enter);
                } else if (e.key.keysym.sym == SDLK_LEFT) {
                    Input::the().set_key(KeyCode::Left);
                } else if (e.key.keysym.sym == SDLK_RIGHT) {
                    Input::the().set_key(KeyCode::Right);
                } else if (e.key.keysym.sym == SDLK_UP) {
                    Input::the().set_key(KeyCode::Up);
                } else if (e.key.keysym.sym == SDLK_DOWN) {
                    Input::the().set_key(KeyCode::Down);
                } else if (e.key.keysym.sym == SDLK_HOME) {
                    Input::the().set_key(KeyCode::Home);
                } else if (e.key.keysym.sym == SDLK_END) {
                    Input::the().set_key(KeyCode::End);
                } else if (e.key.keysym.sym == SDLK_PAGEUP) {
                    Input::the().set_key(KeyCode::PageUp);
                } else if (e.key.keysym.sym == SDLK_PAGEDOWN) {
                    Input::the().set_key(KeyCode::PageDown);
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    Input::the().set_key(KeyCode::Escape);
                } else if (e.key.keysym.sym == SDLK_a && (e.key.keysym.mod & KMOD_CTRL)) {
                    Input::the().set_key((KeyCode)'a');
                } else if (e.key.keysym.sym == SDLK_c && (e.key.keysym.mod & KMOD_CTRL)) {
                    Input::the().set_key((KeyCode)'c');
                } else if (e.key.keysym.sym == SDLK_v && (e.key.keysym.mod & KMOD_CTRL)) {
                    Input::the().set_key((KeyCode)'v');
                } else if (e.key.keysym.sym == SDLK_x && (e.key.keysym.mod & KMOD_CTRL)) {
                    Input::the().set_key((KeyCode)'x');
                }
            }
        }
    }
    return m_running;
}

void DesktopApp::present(Canvas& canvas, std::span<const IntRect> dirty_rects) {
    if (!m_texture || !m_renderer) return;
    if (canvas.width() != static_cast<int>(m_width) || canvas.height() != static_cast<int>(m_height)) return;

    bool full_upload = m_texture_needs_full_upload || dirty_rects.empty();

    if (full_upload) {
        SDL_UpdateTexture(m_texture, nullptr, canvas.pixels(), canvas.width() * static_cast<int>(sizeof(uint32_t)));
        m_texture_needs_full_upload = false;
    } else {
        const int pitch = canvas.width() * static_cast<int>(sizeof(uint32_t));
        for (const auto& rect : dirty_rects) {
            IntRect clipped = rect.intersection({0, 0, canvas.width(), canvas.height()});
            if (clipped.w <= 0 || clipped.h <= 0) continue;

            SDL_Rect sdl_rect{clipped.x, clipped.y, clipped.w, clipped.h};
            const auto* src = reinterpret_cast<const uint8_t*>(canvas.pixels() + clipped.y * canvas.width() + clipped.x);
            SDL_UpdateTexture(m_texture, &sdl_rect, src, pitch);
        }
    }

    SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
    SDL_RenderPresent(m_renderer);
}

void DesktopApp::quit(int exit_code) {
    if (exit_code < 0) {
        std::exit(exit_code);
    }
    m_running = false;
}

void DesktopApp::show() {
    if (m_window) {
        SDL_ShowWindow(m_window);
    }
}

}
