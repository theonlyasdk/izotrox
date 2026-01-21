#include "Canvas.hpp"
#include <cstring>

namespace Izo {

Canvas::Canvas(int width, int height) 
    : m_width(width), m_height(height), m_owns_memory(true) {
    m_pixels = new uint32_t[width * height];
}

Canvas::Canvas(int width, int height, uint32_t* pixels)
    : m_width(width), m_height(height), m_pixels(pixels), m_owns_memory(false) {
}

Canvas::~Canvas() {
    if (m_owns_memory && m_pixels) {
        delete[] m_pixels;
    }
}

size_t Canvas::size_bytes() const {
    return m_width * m_height * sizeof(uint32_t);
}

void Canvas::clear(Color color) {
    uint32_t c = color.as_argb();
    if (c == 0) {
        std::memset(m_pixels, 0, m_width * m_height * sizeof(uint32_t));
    } else {
        size_t count = m_width * m_height;
        for (size_t i = 0; i < count; ++i) {
            m_pixels[i] = c;
        }
    }
}

void Canvas::set_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        m_pixels[y * m_width + x] = color;
    }
}

uint32_t Canvas::pixel(int x, int y) const {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        return m_pixels[y * m_width + x];
    }
    return 0;
}

} // namespace Izo