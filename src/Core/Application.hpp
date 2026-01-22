#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include "Graphics/Canvas.hpp"

namespace Izo {

class Application {
public:
    Application(int width, int height, const char* title);
    ~Application();

    // Returns false if initialization failed
    bool init();

    // Returns false if the application should quit
    bool pump_events();

    // Present the canvas to the screen
    // On Android it copies canvas to framebuffer and swaps doublebuffer
    void present(Canvas& canvas);

    uint32_t width() const;
    uint32_t height() const;

    void on_resize(std::function<void(int, int)> callback);
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace Izo
