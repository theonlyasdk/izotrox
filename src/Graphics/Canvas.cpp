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

void Canvas::set_pixel(IntPoint point, uint32_t color) {
    if (point.x >= 0 && point.x < m_width && point.y >= 0 && point.y < m_height) {
        m_pixels[point.y * m_width + point.x] = color;
    }
}

uint32_t Canvas::pixel(IntPoint point) const {
    if (point.x >= 0 && point.x < m_width && point.y >= 0 && point.y < m_height) {
        return m_pixels[point.y * m_width + point.x];
    }
    return 0;
}

void Canvas::resize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    if (m_width == width && m_height == height) return;

    if (m_owns_memory) {
        delete[] m_pixels;
        m_pixels = new uint32_t[width * height];
    }

    m_width = width;
    m_height = height;
}

} 