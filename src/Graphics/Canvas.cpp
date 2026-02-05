#include "Graphics/Canvas.hpp"
#include <cstring>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../Lib/stb_image_write.h"

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

bool Canvas::save_to_file(const std::string& path) {
    std::vector<uint32_t> rgba(m_width * m_height);
    for (size_t i = 0; i < m_width * m_height; ++i) {
        uint32_t p = m_pixels[i];
        // ARGB -> RGBA
        // A = (p >> 24) & 0xFF
        // R = (p >> 16) & 0xFF
        // G = (p >> 8) & 0xFF
        // B = p & 0xFF
        // RGBA (Little Endian): R G B A in memory -> 0xAABBGGRR in uint32
        
        uint8_t a = (p >> 24) & 0xFF;
        uint8_t r = (p >> 16) & 0xFF;
        uint8_t g = (p >> 8) & 0xFF;
        uint8_t b = p & 0xFF;
        
        rgba[i] = (a << 24) | (b << 16) | (g << 8) | r;
    }
    
    return stbi_write_png(path.c_str(), m_width, m_height, 4, rgba.data(), m_width * 4) != 0;
}

} // namespace Izo