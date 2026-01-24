// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Painter.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace Izo {

Painter::Painter(Canvas& canvas) : m_canvas(&canvas), m_clip_rect{0, 0, canvas.width(), canvas.height()} {}

void Painter::set_canvas(Canvas& canvas) {
    m_canvas = &canvas;
    m_clip_rect = {0, 0, canvas.width(), canvas.height()};
    m_clip_stack.clear();
    m_tx = 0;
    m_ty = 0;
    m_translate_stack.clear();
}

void Painter::push_clip(const IntRect& rect) {
    m_clip_stack.push_back(m_clip_rect);
    IntRect r = {rect.x + m_tx, rect.y + m_ty, rect.w, rect.h};
    m_clip_rect = r.intersection(m_clip_rect);
}

void Painter::pop_clip() {
    if (!m_clip_stack.empty()) {
        m_clip_rect = m_clip_stack.back();
        m_clip_stack.pop_back();
    }
}

void Painter::push_translate(IntPoint offset) {
    m_translate_stack.push_back({m_tx, m_ty});
    m_tx += offset.x;
    m_ty += offset.y;
}

void Painter::pop_translate() {
    if (!m_translate_stack.empty()) {
        m_tx = m_translate_stack.back().x;
        m_ty = m_translate_stack.back().y;
        m_translate_stack.pop_back();
    }
}

void Painter::draw_pixel(IntPoint point, Color color) {
    int dx = point.x + m_tx;
    int dy = point.y + m_ty;
    
    if (dx >= m_clip_rect.x && dx < m_clip_rect.right() && 
        dy >= m_clip_rect.y && dy < m_clip_rect.bottom()) {
        
        if (color.a == 0) return;

        if (color.a == 255) {
            m_canvas->set_pixel({dx, dy}, color.as_argb());
        } else {
            uint32_t bg = m_canvas->pixel({dx, dy});
            
            // Fast fixed-point alpha blending
            uint32_t a = color.a;
            uint32_t inv_a = 256 - a;
            
#ifdef __ANDROID__
            Color bgC(bg);
            uint32_t r = (color.r * a + bgC.r * inv_a) >> 8;
            uint32_t g = (color.g * a + bgC.g * inv_a) >> 8;
            uint32_t b = (color.b * a + bgC.b * inv_a) >> 8;
            m_canvas->set_pixel({dx, dy}, Color((uint8_t)r, (uint8_t)g, (uint8_t)b).as_argb());
#else
            uint32_t rb = bg & 0xFF00FF;
            uint32_t g  = bg & 0x00FF00;
            
            uint32_t color_rb = color.as_argb() & 0xFF00FF;
            uint32_t color_g  = color.as_argb() & 0x00FF00;
            
            uint32_t res_rb = ((color_rb * a + rb * inv_a) >> 8) & 0xFF00FF;
            uint32_t res_g  = ((color_g * a + g * inv_a) >> 8) & 0x00FF00;
            
            m_canvas->set_pixel({dx, dy}, 0xFF000000 | res_rb | res_g);
#endif
        }
    }
}

void Painter::fill_rect(const IntRect& rect, Color color) {
    IntRect r = {rect.x + m_tx, rect.y + m_ty, rect.w, rect.h};
    IntRect dest = r.intersection(m_clip_rect);
    
    if (dest.w <= 0 || dest.h <= 0) return;
    
    uint32_t c = color.as_argb();
    
    if (color.a == 255) {
        for (int iy = 0; iy < dest.h; ++iy) {
            uint32_t* row = m_canvas->pixels() + (dest.y + iy) * m_canvas->width() + dest.x;
            for (int ix = 0; ix < dest.w; ++ix) {
                row[ix] = c;
            }
        }
    } else if (color.a > 0) {
        uint32_t a = color.a;
        uint32_t inv_a = 256 - a;
        
        for (int iy = 0; iy < dest.h; ++iy) {
            int py = dest.y + iy;
            for (int ix = 0; ix < dest.w; ++ix) {
                int px = dest.x + ix;
                uint32_t bg = m_canvas->pixel({px, py});
                
#ifdef __ANDROID__
                Color bgC(bg);
                uint32_t nr = (color.r * a + bgC.r * inv_a) >> 8;
                uint32_t ng = (color.g * a + bgC.g * inv_a) >> 8;
                uint32_t nb = (color.b * a + bgC.b * inv_a) >> 8;
                m_canvas->set_pixel({px, py}, Color((uint8_t)nr, (uint8_t)ng, (uint8_t)nb).as_argb());
#else
                uint32_t rb = bg & 0xFF00FF;
                uint32_t g  = bg & 0x00FF00;
                uint32_t color_rb = c & 0xFF00FF;
                uint32_t color_g  = c & 0x00FF00;
                uint32_t res_rb = ((color_rb * a + rb * inv_a) >> 8) & 0xFF00FF;
                uint32_t res_g  = ((color_g * a + g * inv_a) >> 8) & 0x00FF00;
                m_canvas->set_pixel({px, py}, 0xFF000000 | res_rb | res_g);
#endif
            }
        }
    }
}

void Painter::clear_rect(const IntRect& rect, Color color) {
    IntRect r = {rect.x + m_tx, rect.y + m_ty, rect.w, rect.h};
    IntRect dest = r.intersection(m_clip_rect);
    
    if (dest.w <= 0 || dest.h <= 0) return;
    
    uint32_t c = color.as_argb();
    
    // Direct memory set/copy without alpha blending
    for (int iy = 0; iy < dest.h; ++iy) {
        uint32_t* row = m_canvas->pixels() + (dest.y + iy) * m_canvas->width() + dest.x;
        for (int ix = 0; ix < dest.w; ++ix) {
            row[ix] = c;
        }
    }
}

void Painter::draw_rect(const IntRect& rect, Color color) {
    fill_rect({rect.x, rect.y, rect.w, 1}, color);
    fill_rect({rect.x, rect.y + rect.h - 1, rect.w, 1}, color);
    fill_rect({rect.x, rect.y, 1, rect.h}, color);
    fill_rect({rect.x + rect.w - 1, rect.y, 1, rect.h}, color);
}

void Painter::draw_line(IntPoint p1, IntPoint p2, Color color) {
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    int dx = abs(x2 - x1);
    int dy = -abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    
    while (true) {
        draw_pixel({x1, y1}, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

static void draw_corner_aa(Painter& p, IntPoint center, int radius, int quad, Color color, bool filled, int thickness = 1) {
    int cx = center.x;
    int cy = center.y;
    int x_start = (quad == 0 || quad == 2) ? cx - radius : cx;
    int x_end = (quad == 0 || quad == 2) ? cx : cx + radius;
    int y_start = (quad == 0 || quad == 1) ? cy - radius : cy;
    int y_end = (quad == 0 || quad == 1) ? cy : cy + radius;
    
    // Expand a bit for AA
    if (quad == 0 || quad == 2) x_start--; else x_end++;
    if (quad == 0 || quad == 1) y_start--; else y_end++;

    float r = (float)radius;

    for (int y = y_start; y <= y_end; y++) {
        for (int x = x_start; x <= x_end; x++) {
            float dx = (float)x - (float)cx;
            float dy = (float)y - (float)cy;
            float dist_sq = dx*dx + dy*dy;
            float dist = sqrtf(dist_sq);
            
            if (filled) {
                if (dist <= r - 0.5f) {
                    p.draw_pixel({x, y}, color);
                } else if (dist < r + 0.5f) {
                    float alpha = 1.0f - (dist - (r - 0.5f));
                    Color c = color;
                    c.a = (uint8_t)(c.a * std::clamp(alpha, 0.0f, 1.0f));
                    p.draw_pixel({x, y}, c);
                }
            } else {
                 // Thick outline
                 float t = (float)thickness;
                 float half_t = t / 2.0f;
                 float center_r = r - half_t;
                 float d = fabsf(dist - center_r);
                 
                 if (d < half_t - 0.5f) {
                     p.draw_pixel({x, y}, color);
                 } else if (d < half_t + 0.5f) {
                     float alpha = 1.0f - (d - (half_t - 0.5f));
                     Color c = color;
                     c.a = (uint8_t)(c.a * std::clamp(alpha, 0.0f, 1.0f));
                     p.draw_pixel({x, y}, c);
                 }
            }
        }
    }
}

void Painter::fill_rounded_rect(const IntRect& rect, int radius, Color color) {
    if (radius <= 0) {
        fill_rect(rect, color);
        return;
    }
    
    int min_side = std::min(rect.w, rect.h);
    if (radius * 2 > min_side) radius = min_side / 2;

    fill_rect({rect.x + radius, rect.y, rect.w - 2 * radius, rect.h}, color);
    fill_rect({rect.x, rect.y + radius, radius, rect.h - 2 * radius}, color);
    fill_rect({rect.x + rect.w - radius, rect.y + radius, radius, rect.h - 2 * radius}, color);
    
    draw_corner_aa(*this, {rect.x + radius, rect.y + radius}, radius, 0, color, true);
    draw_corner_aa(*this, {rect.x + rect.w - radius, rect.y + radius}, radius, 1, color, true);
    draw_corner_aa(*this, {rect.x + radius, rect.y + rect.h - radius}, radius, 2, color, true);
    draw_corner_aa(*this, {rect.x + rect.w - radius, rect.y + rect.h - radius}, radius, 3, color, true);
}

void Painter::draw_rounded_rect(const IntRect& rect, int radius, Color color, int thickness) {
    if (radius <= 0) {
        // Draw thick rect using 4 fills
        fill_rect({rect.x, rect.y, rect.w, thickness}, color);
        fill_rect({rect.x, rect.y + rect.h - thickness, rect.w, thickness}, color);
        fill_rect({rect.x, rect.y + thickness, thickness, rect.h - thickness * 2}, color);
        fill_rect({rect.x + rect.w - thickness, rect.y + thickness, thickness, rect.h - thickness * 2}, color);
        return;
    }

    int min_side = std::min(rect.w, rect.h);
    if (radius * 2 > min_side) radius = min_side / 2;
    
    // Sides
    fill_rect({rect.x + radius, rect.y, rect.w - 2 * radius, thickness}, color);
    fill_rect({rect.x + radius, rect.y + rect.h - thickness, rect.w - 2 * radius, thickness}, color);
    fill_rect({rect.x, rect.y + radius, thickness, rect.h - 2 * radius}, color);
    fill_rect({rect.x + rect.w - thickness, rect.y + radius, thickness, rect.h - 2 * radius}, color);

    // Corners
    draw_corner_aa(*this, {rect.x + radius, rect.y + radius}, radius, 0, color, false, thickness);
    draw_corner_aa(*this, {rect.x + rect.w - radius, rect.y + radius}, radius, 1, color, false, thickness);
    draw_corner_aa(*this, {rect.x + radius, rect.y + rect.h - radius}, radius, 2, color, false, thickness);
    draw_corner_aa(*this, {rect.x + rect.w - radius, rect.y + rect.h - radius}, radius, 3, color, false, thickness);
}

} // namespace Izo