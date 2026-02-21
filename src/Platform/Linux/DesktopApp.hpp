#pragma once

#include <SDL.h>
#include <cstdint>
#include <functional>
#include <string>

#include "Core/AppImplementation.hpp"

namespace Izo {

class DesktopApp final : public AppImplementation {
public:
    DesktopApp(std::string caption, int width = 800, int height = 600);
    ~DesktopApp() override;

    bool init() override;
    bool pump_events() override;
    void present(Canvas& canvas) override;
    void quit(int exit_code) override;
    void show() override;

    uint32_t width() const override { return m_width; }
    uint32_t height() const override { return m_height; }

    void set_on_resize(std::function<void(int, int)> cb) override { m_on_resize = std::move(cb); }

private:
    bool recreate_texture(uint32_t w, uint32_t h);

    std::string m_caption;
    SDL_Window* m_window{nullptr};
    SDL_Renderer* m_renderer{nullptr};
    SDL_Texture* m_texture{nullptr};
    bool m_running{false};
    uint32_t m_width{0};
    uint32_t m_height{0};

    std::function<void(int, int)> m_on_resize;
};

}
