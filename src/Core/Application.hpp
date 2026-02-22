#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <span>

#include "Core/AppImplementation.hpp"
#include "Geometry/Primitives.hpp"

namespace Izo {

class Canvas;

class Application {
public:
    static Application& the() noexcept { return *_instance; }

    Application(int width, int height, const char* title);
    ~Application();

    bool init();

    bool pump_events();

    void present(Canvas& canvas, std::span<const IntRect> dirty_rects = {});

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    const IntRect screen_rect() const {
        return IntRect{0, 0, static_cast<int>(width()), static_cast<int>(height())};
    }

    void set_debug(bool flag) { m_debug = flag; }
    const bool debug_mode() const { return m_debug; }

    float delta() const { return m_delta; }
    void set_delta(float dt) { m_delta = dt; }

    void quit(int exit_code);
    void show();
    void on_resize(std::function<void(int, int)> callback);

private:
    float m_delta{0.f};
    bool m_debug{false};
    uint32_t m_width{0};
    uint32_t m_height{0};
    std::function<void(int, int)> m_on_resize;
    std::unique_ptr<AppImplementation> m_backend{nullptr};
    static Application* _instance;
};

}
