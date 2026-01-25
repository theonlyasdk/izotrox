#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include "Graphics/Canvas.hpp"

namespace Izo {

class Application {
public:
    static Application& the() noexcept { return *_instance; }

    Application(int width, int height, const char* title);
    ~Application();

    bool init();

    bool pump_events();

    void present(Canvas& canvas);

    uint32_t width() const;
    uint32_t height() const;

    float delta() const noexcept { return _delta; }
    void set_delta(float dt) noexcept { _delta = dt; }

    void quit();
    void on_resize(std::function<void(int, int)> callback);
private:
    float _delta{0.f};

    struct Impl;
    std::unique_ptr<Impl> impl;
    static Application* _instance;
};

} 
