#include "Painter.hpp"
#include <cmath>
#include <algorithm>

namespace Izo {

Painter::Painter(Canvas& canvas) : canvas_(canvas) {
    reset_clip();
}

void Painter::set_clip(int x, int y, int w, int h) {
    Rect r = {x, y, w, h};
    Rect screen = {0, 0, canvas_.width(), canvas_.height()};
    clip_rect_ = r.intersection(screen);
}

void Painter::reset_clip() {
    clip_rect_ = {0, 0, canvas_.width(), canvas_.height()};
}

void Painter::draw_pixel(int x, int y, Color color) {
    if (x >= clip_rect_.x && x < clip_rect_.right() &&
        y >= clip_rect_.y && y < clip_rect_.bottom()) {
        canvas_.set_pixel(x, y, color.to_argb());
    }
}

void Painter::fill_rect(int x, int y, int w, int h, Color color) {
    Rect r = {x, y, w, h};
    Rect draw_rect = r.intersection(clip_rect_);
    
    if (draw_rect.w <= 0 || draw_rect.h <= 0) return;

    uint32_t c = color.to_argb();
    uint32_t* pixels = canvas_.pixels();
    int width = canvas_.width();

    for (int cy = draw_rect.y; cy < draw_rect.bottom(); ++cy) {
        uint32_t* row = pixels + cy * width + draw_rect.x;
        // Optimization: if simple color, maybe memset?
        // But 32-bit color memset is not standard unless 0 or -1.
        for (int cx = 0; cx < draw_rect.w; ++cx) {
            row[cx] = c;
        }
    }
}

void Painter::draw_rect(int x, int y, int w, int h, Color color) {
    // Top
    fill_rect(x, y, w, 1, color);
    // Bottom
    fill_rect(x, y + h - 1, w, 1, color);
    // Left
    fill_rect(x, y + 1, 1, h - 2, color); // +1/-2 to avoid overdrawing corners
    // Right
    fill_rect(x + w - 1, y + 1, 1, h - 2, color);
}

void Painter::draw_line(int x0, int y0, int x1, int y1, Color color) {
    // Bresenham's line algorithm
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

} // namespace Izo
