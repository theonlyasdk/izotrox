#pragma once

#ifdef __ANDROID__

#include <functional>
#include <string>

#include "Core/AppImplementation.hpp"
#include "HAL/Framebuffer.hpp"

namespace Izo {

class AndroidApp final : public AppImplementation {
public:
    AndroidApp(int width, int height, std::string title);
    ~AndroidApp() override;

    bool init() override;
    bool pump_events() override;
    void present(Canvas& canvas, std::span<const IntRect> dirty_rects) override;
    void quit(int exit_code) override;
    void show() override;

    uint32_t width() const override { return m_width; }
    uint32_t height() const override { return m_height; }

    void set_on_resize(std::function<void(int, int)> cb) override { m_on_resize = std::move(cb); }

private:
    Framebuffer m_fb;
    uint32_t m_width;
    uint32_t m_height;
    std::function<void(int, int)> m_on_resize;
};

}

#endif
