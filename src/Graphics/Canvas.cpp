#include "Canvas.hpp"
#include <cstring>

namespace Izo {

Canvas::Canvas(int width, int height) 
    : width_(width), height_(height), owns_memory_(true) {
    pixels_ = new uint32_t[width * height];
}

Canvas::Canvas(int width, int height, uint32_t* pixels)
    : width_(width), height_(height), pixels_(pixels), owns_memory_(false) {
}

Canvas::~Canvas() {
    if (owns_memory_ && pixels_) {
        delete[] pixels_;
    }
}

size_t Canvas::size_bytes() const {
    return width_ * height_ * sizeof(uint32_t);
}

void Canvas::clear(Color color) {
    uint32_t c = color.to_argb();
    if (c == 0) {
        std::memset(pixels_, 0, width_ * height_ * sizeof(uint32_t));
    } else {
        size_t count = width_ * height_;
        for (size_t i = 0; i < count; ++i) {
            pixels_[i] = c;
        }
    }
}

void Canvas::set_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        pixels_[y * width_ + x] = color;
    }
}

uint32_t Canvas::pixel(int x, int y) const {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        return pixels_[y * width_ + x];
    }
    return 0;
}

} // namespace Izo