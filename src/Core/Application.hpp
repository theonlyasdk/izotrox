#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include "Geometry/Primitives.hpp"
#include "Platform/Linux/SDLApplication.hpp"

#ifdef __ANDROID__
#include "HAL/Framebuffer.hpp"
#endif

namespace Izo {

class Canvas;

class Application {
public:
    struct Impl {
        int width, height;
        std::function<void(int, int)> on_resize;

#ifdef __ANDROID__
            Framebuffer fb;
#else
            std::unique_ptr<SDLApplication> sdl_app = nullptr;
#endif

        Impl(int w, int h, const char* title) : width(w), height(h) {
#ifdef __ANDROID__
                sdl_app = std::make_unique<SDLApplication>(title, w, h);
#endif
        }
    };

    static Application& the() noexcept { return *_instance; }

    Application(int width, int height, const char* title);
    ~Application();

    bool init();

    bool pump_events();

    void present(Canvas& canvas);

    const uint32_t width() const;
    const uint32_t height() const;
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
    friend class Painter;

    float m_delta{0.f};
    bool m_debug;

    std::unique_ptr<Impl> impl = nullptr;
    static Application* _instance;
};

}
