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
            
            // Fast fixed-point alpha blending
            uint32_t a = color.a;
            uint32_t inv_a = 255 - a;
            
#ifdef __ANDROID__
            Color bgC(bg);
            uint32_t r = (color.r * a + bgC.r * inv_a) >> 8;
            uint32_t g = (color.g * a + bgC.g * inv_a) >> 8;
            uint32_t b = (color.b * a + bgC.b * inv_a) >> 8;
            m_canvas_ref.set_pixel(x, y, Color((uint8_t)r, (uint8_t)g, (uint8_t)b).to_argb());
#else
            uint32_t rb = bg & 0xFF00FF;
            uint32_t g  = bg & 0x00FF00;
            
            uint32_t color_rb = color.to_argb() & 0xFF00FF;
            uint32_t color_g  = color.to_argb() & 0x00FF00;
            
            uint32_t res_rb = ((color_rb * a + rb * inv_a) >> 8) & 0xFF00FF;
            uint32_t res_g  = ((color_g * a + g * inv_a) >> 8) & 0x00FF00;
            
            m_canvas_ref.set_pixel(x, y, 0xFF000000 | res_rb | res_g);
#endif
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
            uint32_t* row = m_canvas_ref.pixels() + (dest.y + iy) * m_canvas_ref.width() + dest.x;
            for (int ix = 0; ix < dest.w; ++ix) {
                row[ix] = c;
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

static void draw_corner_aa(Painter& p, int cx, int cy, int radius, int quad, Color color, bool filled, int thickness = 1) {
    int x_start = (quad == 0 || quad == 2) ? cx - radius : cx;
    int x_end = (quad == 0 || quad == 2) ? cx : cx + radius;
    int y_start = (quad == 0 || quad == 1) ? cy - radius : cy;
    int y_end = (quad == 0 || quad == 1) ? cy : cy + radius;
    
    // Expand a bit for AA
    if (quad == 0 || quad == 2) x_start--; else x_end++;
    if (quad == 0 || quad == 1) y_start--; else y_end++;

    float r = (float)radius;
    float rSq = r * r;

    for (int y = y_start; y <= y_end; y++) {
        for (int x = x_start; x <= x_end; x++) {
            float dx = (float)x - (float)cx;
            float dy = (float)y - (float)cy;
            float dist_sq = dx*dx + dy*dy;
            float dist = sqrtf(dist_sq);
            
            if (filled) {
                if (dist <= r - 0.5f) {
                    p.draw_pixel(x, y, color);
                } else if (dist < r + 0.5f) {
                    float alpha = 1.0f - (dist - (r - 0.5f));
                    Color c = color;
                    c.a = (uint8_t)(c.a * std::clamp(alpha, 0.0f, 1.0f));
                    p.draw_pixel(x, y, c);
                }
            } else {
                 // Thick outline
                 float t = (float)thickness;
                 float half_t = t / 2.0f;
                 float center_r = r - half_t;
                 float d = fabsf(dist - center_r);
                 
                 if (d < half_t - 0.5f) {
                     p.draw_pixel(x, y, color);
                 } else if (d < half_t + 0.5f) {
                     float alpha = 1.0f - (d - (half_t - 0.5f));
                     Color c = color;
                     c.a = (uint8_t)(c.a * std::clamp(alpha, 0.0f, 1.0f));
                     p.draw_pixel(x, y, c);
                 }
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

void Painter::draw_rounded_rect(int x, int y, int w, int h, int radius, Color color, int thickness) {
    if (radius <= 0) {
        // Draw thick rect using 4 fills
        fill_rect(x, y, w, thickness, color);
        fill_rect(x, y + h - thickness, w, thickness, color);
        fill_rect(x, y + thickness, thickness, h - thickness * 2, color);
        fill_rect(x + w - thickness, y + thickness, thickness, h - thickness * 2, color);
        return;
    }

    int min_side = std::min(w, h);
    if (radius * 2 > min_side) radius = min_side / 2;
    
    // Sides
    fill_rect(x + radius, y, w - 2 * radius, thickness, color);
    fill_rect(x + radius, y + h - thickness, w - 2 * radius, thickness, color);
    fill_rect(x, y + radius, thickness, h - 2 * radius, color);
    fill_rect(x + w - thickness, y + radius, thickness, h - 2 * radius, color);

    // Corners
    draw_corner_aa(*this, x + radius, y + radius, radius, 0, color, false, thickness);
    draw_corner_aa(*this, x + w - radius, y + radius, radius, 1, color, false, thickness);
    draw_corner_aa(*this, x + radius, y + h - radius, radius, 2, color, false, thickness);
    draw_corner_aa(*this, x + w - radius, y + h - radius, radius, 3, color, false, thickness);
}

} // namespace Izo