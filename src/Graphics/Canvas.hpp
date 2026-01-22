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

    int width() const { return m_width; }
    int height() const { return m_height; }
    
    uint32_t* pixels() { return m_pixels; }
    const uint32_t* pixels() const { return m_pixels; }
    
    size_t size_bytes() const;

    void clear(Color color);

    void set_pixel(int x, int y, uint32_t color);

    uint32_t pixel(int x, int y) const;

    void resize(int width, int height);

private:
    int m_width;
    int m_height;
    uint32_t* m_pixels;
    bool m_owns_memory;
};

} // namespace Izo