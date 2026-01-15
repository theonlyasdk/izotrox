// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Painter.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace Izo {

Painter::Painter(Canvas& canvas) : m_canvas_ref(canvas), m_clip_rect{0, 0, canvas.width(), canvas.height()} {}

void Painter::set_clip(int x, int y, int w, int h) {
    IntRect r = {x, y, w, h};
    IntRect screen = {0, 0, m_canvas_ref.width(), m_canvas_ref.height()};
    m_clip_rect = r.intersection(screen);
}

void Painter::reset_clip() {
    m_clip_rect = {0, 0, m_canvas_ref.width(), m_canvas_ref.height()};
}

void Painter::draw_pixel(int x, int y, Color color) {
    if (x >= m_clip_rect.x && x < m_clip_rect.right() && 
        y >= m_clip_rect.y && y < m_clip_rect.bottom()) {
        
        if (color.a == 0) return;

        if (color.a == 255) {
            m_canvas_ref.set_pixel(x, y, color.to_argb());
        } else {
            uint32_t bg = m_canvas_ref.pixel(x, y);
            Color bgC(bg);
            
            float alpha = color.a / 255.0f;
            float inv = 1.0f - alpha;
            
            uint8_t r = (uint8_t)(color.r * alpha + bgC.r * inv);
            uint8_t g = (uint8_t)(color.g * alpha + bgC.g * inv);
            uint8_t b = (uint8_t)(color.b * alpha + bgC.b * inv);
            
            m_canvas_ref.set_pixel(x, y, Color(r, g, b).to_argb());
        }
    }
}

void Painter::fill_rect(int x, int y, int w, int h, Color color) {
    IntRect r = {x, y, w, h};
    IntRect dest = r.intersection(m_clip_rect);
    
    if (dest.w <= 0 || dest.h <= 0) return;
    
    uint32_t c = color.to_argb();
    
    if (color.a == 255) {
        for (int iy = 0; iy < dest.h; ++iy) {
            for (int ix = 0; ix < dest.w; ++ix) {
                m_canvas_ref.set_pixel(dest.x + ix, dest.y + iy, c);
            }
        }
    } else if (color.a > 0) {
        for (int iy = 0; iy < dest.h; ++iy) {
            for (int ix = 0; ix < dest.w; ++ix) {
                draw_pixel(dest.x + ix, dest.y + iy, color);
            }
        }
    }
}

void Painter::draw_rect(int x, int y, int w, int h, Color color) {
    fill_rect(x, y, w, 1, color);
    fill_rect(x, y + h - 1, w, 1, color);
    fill_rect(x, y, 1, h, color);
    fill_rect(x + w - 1, y, 1, h, color);
}

void Painter::draw_line(int x1, int y1, int x2, int y2, Color color) {
    int dx = abs(x2 - x1);
    int dy = -abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    
    while (true) {
        draw_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

static void draw_corner_aa(Painter& p, int cx, int cy, int radius, int quad, Color color, bool filled) {
    int x_start = (quad == 0 || quad == 2) ? cx - radius : cx;
    int x_end = (quad == 0 || quad == 2) ? cx : cx + radius;
    int y_start = (quad == 0 || quad == 1) ? cy - radius : cy;
    int y_end = (quad == 0 || quad == 1) ? cy : cy + radius;
    
    if (quad == 0 || quad == 2) x_start--; else x_end++;
    if (quad == 0 || quad == 1) y_start--; else y_end++;

    float rSq = (float)(radius * radius);
    float r = (float)radius;

    for (int y = y_start; y <= y_end; y++) {
        for (int x = x_start; x <= x_end; x++) {
            float dx = (float)x - (float)cx;
            float dy = (float)y - (float)cy;
            float dist = sqrtf((dx + 0.5f)*(dx + 0.5f) + (dy + 0.5f)*(dy + 0.5f));
            float alpha = 0.0f;
            
            if (filled) {
                 alpha = 1.0f - (dist - (r - 0.5f));
            } else {
                 float width = 1.2f;
                 float offset = 0.6f; 
                 float d = fabsf(dist - (r - offset));
                 if (d < width) {
                     alpha = 1.0f - (d / width);
                     alpha = std::min(1.0f, alpha * 1.5f);
                 }
            }

            if (alpha < 0.0f) alpha = 0.0f;
            if (alpha > 1.0f) alpha = 1.0f;
            
            if (alpha > 0.0f) {
                Color c = color;
                c.a = (uint8_t)(c.a * alpha);
                p.draw_pixel(x, y, c);
            }
        }
    }
}

void Painter::fill_rounded_rect(int x, int y, int w, int h, int radius, Color color) {
    if (radius <= 0) {
        fill_rect(x, y, w, h, color);
        return;
    }
    
    int min_side = std::min(w, h);
    if (radius * 2 > min_side) radius = min_side / 2;

    fill_rect(x + radius, y, w - 2 * radius, h, color);
    fill_rect(x, y + radius, radius, h - 2 * radius, color);
    fill_rect(x + w - radius, y + radius, radius, h - 2 * radius, color);
    
    draw_corner_aa(*this, x + radius, y + radius, radius, 0, color, true);
    draw_corner_aa(*this, x + w - radius, y + radius, radius, 1, color, true);
    draw_corner_aa(*this, x + radius, y + h - radius, radius, 2, color, true);
    draw_corner_aa(*this, x + w - radius, y + h - radius, radius, 3, color, true);
}

void Painter::draw_rounded_rect(int x, int y, int w, int h, int radius, Color color) {
    if (radius <= 0) {
        draw_rect(x, y, w, h, color);
        return;
    }

    int min_side = std::min(w, h);
    if (radius * 2 > min_side) radius = min_side / 2;
    
    fill_rect(x + radius, y, w - 2 * radius, 1, color);
    fill_rect(x + radius, y + h - 1, w - 2 * radius, 1, color);
    fill_rect(x, y + radius, 1, h - 2 * radius, color);
    fill_rect(x + w - 1, y + radius, 1, h - 2 * radius, color);

    draw_corner_aa(*this, x + radius, y + radius, radius, 0, color, false);
    draw_corner_aa(*this, x + w - radius, y + radius, radius, 1, color, false);
    draw_corner_aa(*this, x + radius, y + h - radius, radius, 2, color, false);
    draw_corner_aa(*this, x + w - radius, y + h - radius, radius, 3, color, false);
}

} // namespace Izo