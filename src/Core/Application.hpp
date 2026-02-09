#pragma once

#include <cstdint>
#include <functional>

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

    void present(Canvas& canvas);

    const uint32_t width() const;
    const uint32_t height() const;
    const IntRect screen_rect() const;

    void set_debug(bool flag) { m_debug = flag; }
    const bool debug_mode() const { return m_debug; }

    float delta() const { return _delta; }
    void set_delta(float dt) { _delta = dt; }

    void quit(int exit_code);
    void show();
    void on_resize(std::function<void(int, int)> callback);
private:
    float _delta{0.f};
    bool m_debug;

    struct Impl;
    std::unique_ptr<Impl> impl;
    static Application* _instance;
};

}
