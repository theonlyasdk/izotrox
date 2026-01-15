#pragma once
#include <memory>
#include <functional>
#include "../Graphics/Canvas.hpp"

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
    void present(Canvas& canvas);

    int width() const;
    int height() const;

    void set_on_resize(std::function<void(int, int)> callback);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace Izo
