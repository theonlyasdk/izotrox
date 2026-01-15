#pragma once
#include "Color.hpp"
#include <vector>
#include <memory>
#include <span>

namespace Izo {

class Canvas {
public:
    // Create a canvas that owns its memory
    Canvas(int width, int height);

    // Create a canvas that wraps existing memory (e.g. Framebuffer)
    // The memory is NOT freed when this canvas is destroyed.
    Canvas(int width, int height, uint32_t* pixels);

    ~Canvas();

    int width() const { return width_; }
    int height() const { return height_; }
    
    uint32_t* pixels() { return pixels_; }
    const uint32_t* pixels() const { return pixels_; }
    
    size_t size_bytes() const;

    void clear(Color color);

    void set_pixel(int x, int y, uint32_t color);

    uint32_t pixel(int x, int y) const;

private:
    int width_;
    int height_;
    uint32_t* pixels_;
    bool owns_memory_;
};

} // namespace Izo