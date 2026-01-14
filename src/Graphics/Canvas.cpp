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

void Canvas::clear(Color color) {
    uint32_t c = color.to_argb();
    // Optimization for black/zero
    if (c == 0) {
        std::memset(pixels_, 0, width_ * height_ * sizeof(uint32_t));
    } else {
        // We can use std::fill_n or a loop
        // std::fill_n might be optimized
        size_t count = width_ * height_;
        for (size_t i = 0; i < count; ++i) {
            pixels_[i] = c;
        }
    }
}

} // namespace Izo
