#pragma once

#include <cstdint>
#include <functional>

#include "Geometry/Primitives.hpp"

namespace Izo {

class Canvas;

class AppImplementation {
public:
    virtual ~AppImplementation() = default;

    virtual bool init() = 0;
    virtual bool pump_events() = 0;
    virtual void present(Canvas& canvas) = 0;
    virtual void quit(int exit_code) = 0;
    virtual void show() = 0;

    virtual uint32_t width() const = 0;
    virtual uint32_t height() const = 0;

    virtual void set_on_resize(std::function<void(int, int)> cb) = 0;
};

}
