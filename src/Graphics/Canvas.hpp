#pragma once
#include "Graphics/Color.hpp"
#include "Geometry/Primitives.hpp"

namespace Izo {

class Canvas {
public:
    Canvas(int width, int height);
    Canvas(int width, int height, uint32_t* pixels);
    Canvas(const Canvas&) = delete;
    Canvas(const Canvas&&) = delete;
    ~Canvas();

    int width() const { return m_width; }
    int height() const { return m_height; }

    uint32_t* pixels() { return m_pixels; }
    const uint32_t* pixels() const { return m_pixels; }

    size_t size_bytes() const { return m_width * m_height * sizeof(uint32_t); }

    void clear(Color color);

    void set_pixel(IntPoint point, uint32_t color);

    uint32_t pixel_at(IntPoint point) const;

    void resize(int width, int height);

    bool save_to_file(const std::string& path);


private:
    int m_width;
    int m_height;
    uint32_t* m_pixels;
    bool m_owns_memory;
};

} 