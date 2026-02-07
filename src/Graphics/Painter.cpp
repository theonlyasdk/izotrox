#include "Graphics/Painter.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "Core/Application.hpp"
#include "Geometry/Primitives.hpp"

namespace Izo {

Painter::Painter(Canvas& canvas) : m_canvas(&canvas) {
    m_current_clip = {{0, 0, m_canvas->width(), m_canvas->height()}, 0};
}

IntRect Painter::apply_translate_to_rect(const IntRect& rect) {
    return IntRect{rect.x + m_translate_x, rect.y + m_translate_y, rect.w, rect.h};
}

void Painter::set_global_alpha(float alpha)  {
    m_global_alpha = std::clamp(alpha, 0.0f, 1.0f);
}

void Painter::set_canvas(Canvas& canvas) {
    m_canvas = &canvas;
    m_current_clip = {{0, 0, m_canvas->width(), m_canvas->height()}, 0};
    m_clip_stack.clear();
    m_translate_stack.clear();
    m_translate_x = 0;
    m_translate_y = 0;
}

void Painter::push_rounded_clip(const IntRect& rect, int radius) {
    m_clip_stack.push_back(m_current_clip);
    IntRect new_rect = apply_translate_to_rect(rect).intersection(m_current_clip.rect);
    m_current_clip = {new_rect, radius};
}

void Painter::push_clip(const IntRect& rect) { 
    push_rounded_clip(rect, 0); 
}

void Painter::pop_clip() {
    if (!m_clip_stack.empty()) {
        m_current_clip = m_clip_stack.back();
        m_clip_stack.pop_back();
    }
}

void Painter::push_translate(IntPoint offset) {
    m_translate_stack.push_back({m_translate_x, m_translate_y});
    m_translate_x += offset.x;
    m_translate_y += offset.y;
}

void Painter::pop_translate() {
    if (!m_translate_stack.empty()) {
        m_translate_x = m_translate_stack.back().x;
        m_translate_y = m_translate_stack.back().y;
        m_translate_stack.pop_back();
    }
}

void Painter::draw_pixel(IntPoint point, Color color) {
    if (point.x < 0 || point.y < 0) return;
    if (point.x > Application::the().width() ||
        point.y > Application::the().height())
        return;

    int dx = point.x + m_translate_x;
    int dy = point.y + m_translate_y;
    const IntRect& cr = m_current_clip.rect;

    if (dx >= cr.x && dx < cr.right() && dy >= cr.y && dy < cr.bottom()) {
        if (m_current_clip.radius > 0) {
            int rx = m_current_clip.rect.x;
            int ry = m_current_clip.rect.y;
            int rw = m_current_clip.rect.w;
            int rh = m_current_clip.rect.h;
            int r = m_current_clip.radius;

            bool in_corner = false;
            int cx = 0, cy = 0;

            if (dx < rx + r && dy < ry + r) {
                cx = rx + r;
                cy = ry + r;
                in_corner = true;
            }  // TL
            else if (dx >= rx + rw - r && dy < ry + r) {
                cx = rx + rw - r;
                cy = ry + r;
                in_corner = true;
            }  // TR
            else if (dx < rx + r && dy >= ry + rh - r) {
                cx = rx + r;
                cy = ry + rh - r;
                in_corner = true;
            }  // BL
            else if (dx >= rx + rw - r && dy >= ry + rh - r) {
                cx = rx + rw - r;
                cy = ry + rh - r;
                in_corner = true;
            }  // BR

            if (in_corner) {
                int dist_sq = (dx - cx + (dx < cx ? 0.5 : 0.5)) * (dx - cx + 0.5) + (dy - cy + 0.5) * (dy - cy + 0.5);

                float fdx = (float)dx + 0.5f;
                float fdy = (float)dy + 0.5f;
                float fcx = (float)cx;
                float fcy = (float)cy;
                float dist = std::sqrt((fdx - fcx) * (fdx - fcx) +
                                       (fdy - fcy) * (fdy - fcy));
                if (dist > (float)r) return;
                // TODO: We could apply AA here too...
            }
        }

        uint8_t final_a = (uint8_t)(color.a * m_global_alpha);
        if (final_a == 0) return;

        if (final_a == 255) {
            m_canvas->set_pixel({dx, dy}, color.as_argb());
        } else {
            uint32_t bg = m_canvas->pixel({dx, dy});
            uint32_t a = final_a;
            uint32_t inv_a = 256 - a;

#ifdef __ANDROID__
            Color bg_color(bg);
            uint32_t r = (color.r * a + bg_color.r * inv_a) >> 8;
            uint32_t g = (color.g * a + bg_color.g * inv_a) >> 8;
            uint32_t b = (color.b * a + bg_color.b * inv_a) >> 8;
            m_canvas->set_pixel({dx, dy}, Color((uint8_t)r, (uint8_t)g, (uint8_t)b).as_argb());
#else
            uint32_t rb = bg & 0xFF00FF;
            uint32_t g = bg & 0x00FF00;
            uint32_t color_rb = color.as_argb() & 0xFF00FF;
            uint32_t color_g = color.as_argb() & 0x00FF00;
            uint32_t res_rb = ((color_rb * a + rb * inv_a) >> 8) & 0xFF00FF;
            uint32_t res_g = ((color_g * a + g * inv_a) >> 8) & 0x00FF00;
            m_canvas->set_pixel({dx, dy}, 0xFF000000 | res_rb | res_g);
#endif
        }
    }
}

void Painter::fill_rect(const IntRect& rect, Color color) {
    if (rect.w < 0 || rect.h < 0) return;

    IntRect dest = apply_translate_to_rect(rect).intersection(m_current_clip.rect);

    if (dest.w <= 0 || dest.h <= 0) return;

    // Apply global alpha
    Color final_color = color;
    final_color.a = (uint8_t)(color.a * m_global_alpha);
    if (final_color.a == 0) return;

    if (m_current_clip.radius > 0) {
        for (int iy = 0; iy < dest.h; ++iy) {
            for (int ix = 0; ix < dest.w; ++ix) {
                draw_pixel({dest.x + ix - m_translate_x, dest.y + iy - m_translate_y}, color);
            }
        }
        return;
    }

    uint32_t c = final_color.as_argb();

    if (final_color.a == 255) {
        for (int iy = 0; iy < dest.h; ++iy) {
            uint32_t* row = m_canvas->pixels() + (dest.y + iy) * m_canvas->width() + dest.x;
            for (int ix = 0; ix < dest.w; ++ix) {
                row[ix] = c;
            }
        }
    } else {
        uint32_t a = final_color.a;
        uint32_t inv_a = 256 - a;

        for (int iy = 0; iy < dest.h; ++iy) {
            int py = dest.y + iy;
            for (int ix = 0; ix < dest.w; ++ix) {
                int px = dest.x + ix;
                uint32_t bg_pixel = m_canvas->pixel({px, py});

#ifdef __ANDROID__
                Color bg_color(bg);
                uint32_t nr = (final_color.r * a + bg_color.r * inv_a) >> 8;
                uint32_t ng = (final_color.g * a + bg_color.g * inv_a) >> 8;
                uint32_t nb = (final_color.b * a + bg_color.b * inv_a) >> 8;
                m_canvas->set_pixel({px, py}, Color((uint8_t)nr, (uint8_t)ng, (uint8_t)nb).as_argb());
#else
                uint32_t rb = bg_pixel & 0xFF00FF;
                uint32_t g = bg_pixel & 0x00FF00;
                uint32_t color_rb = c & 0xFF00FF;
                uint32_t color_g = c & 0x00FF00;
                uint32_t res_rb = ((color_rb * a + rb * inv_a) >> 8) & 0xFF00FF;
                uint32_t res_g = ((color_g * a + g * inv_a) >> 8) & 0x00FF00;
                m_canvas->set_pixel({px, py}, 0xFF000000 | res_rb | res_g);
#endif
            }
        }
    }
}

void Painter::clear_rect(const IntRect& rect, Color color) {
    fill_rect(rect, color);
}

void Painter::outline_rect(const IntRect& rect, Color color) {
    if (rect.w < 0 || rect.h < 0) return;

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
        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}

static void draw_corner(Painter& p, IntPoint center, int radius, int quad,
                        Color color, bool filled, int thickness = 1) {
    int cx = center.x;
    int cy = center.y;
    float r = (float)radius;

    for (int y = 0; y < radius; y++) {
        for (int x = 0; x < radius; x++) {
            float dx = (float)x + 0.5f;
            float dy = (float)y + 0.5f;
            float dist = std::sqrt(dx * dx + dy * dy);

            float alpha = 0.0f;
            if (filled) {
                if (dist <= r - 0.5f) {
                    alpha = 1.0f;
                } else if (dist < r + 0.5f) {
                    alpha = 1.0f - (dist - (r - 0.5f));
                }
            } else {
                float t = (float)thickness;
                float half_t = t / 2.0f;
                float center_r = r - half_t;
                float d = std::abs(dist - center_r);
                if (d < half_t - 0.5f) {
                    alpha = 1.0f;
                } else if (d < half_t + 0.5f) {
                    alpha = 1.0f - (d - (half_t - 0.5f));
                }
            }

            if (alpha > 0.0f) {
                Color c = color;
                c.a = (uint8_t)(c.a * std::clamp(alpha, 0.0f, 1.0f));

                int px = (quad == 0 || quad == 2) ? cx - 1 - x : cx + x;
                int py = (quad == 0 || quad == 1) ? cy - 1 - y : cy + y;

                p.draw_pixel({px, py}, c);
            }
        }
    }
}

void Painter::fill_rounded_rect(const IntRect& rect, int radius, Color color, int corners) {
    if (rect.w <= 0 || rect.h <= 0) return;

    if (radius <= 0) {
        fill_rect(rect, color);
        return;
    }

    int maxR = std::min(rect.w, rect.h) / 2;
    radius = std::min(radius, maxR);

    // Central column
    fill_rect({rect.x + radius, rect.y, rect.w - 2 * radius, rect.h}, color);

    // Left side (between corners)
    fill_rect({rect.x, rect.y + radius, radius, rect.h - 2 * radius}, color);

    // Right side (between corners)
    fill_rect({rect.x + rect.w - radius, rect.y + radius, radius, rect.h - 2 * radius}, color);

    // Top Left
    if (corners & Corner::TopLeft) {
        draw_corner(*this, {rect.x + radius, rect.y + radius}, radius, 0, color, true);
    } else {
        fill_rect({rect.x, rect.y, radius, radius}, color);
    }

    // Top Right
    if (corners & Corner::TopRight) {
        draw_corner(*this, {rect.x + rect.w - radius, rect.y + radius}, radius, 1, color, true);
    } else {
        fill_rect({rect.x + rect.w - radius, rect.y, radius, radius}, color);
    }

    // Bottom Left
    if (corners & Corner::BottomLeft) {
        draw_corner(*this, {rect.x + radius, rect.y + rect.h - radius}, radius, 2, color, true);
    } else {
        fill_rect({rect.x, rect.y + rect.h - radius, radius, radius}, color);
    }

    // Bottom Right
    if (corners & Corner::BottomRight) {
        draw_corner(*this, {rect.x + rect.w - radius, rect.y + rect.h - radius}, radius, 3, color, true);
    } else {
        fill_rect({rect.x + rect.w - radius, rect.y + rect.h - radius, radius, radius}, color);
    }
}

void Painter::draw_rounded_rect(const IntRect& rect, int radius, Color color, int thickness) {
    if (rect.w <= 0 || rect.h <= 0) return;

    if (radius <= 0) {
        fill_rect({rect.x, rect.y, rect.w, thickness}, color);
        fill_rect({rect.x, rect.y + rect.h - thickness, rect.w, thickness}, color);
        fill_rect({rect.x, rect.y + thickness, thickness, rect.h - 2 * thickness}, color);
        fill_rect({rect.x + rect.w - thickness, rect.y + thickness, thickness, rect.h - 2 * thickness}, color);
        return;
    }
    int maxR = std::min(rect.w, rect.h) / 2;
    radius = std::min(radius, maxR);
    fill_rect({rect.x + radius, rect.y, rect.w - 2 * radius, thickness}, color);
    fill_rect({rect.x + radius, rect.y + rect.h - thickness, rect.w - 2 * radius, thickness}, color);
    fill_rect({rect.x, rect.y + radius, thickness, rect.h - 2 * radius}, color);
    fill_rect({rect.x + rect.w - thickness, rect.y + radius, thickness, rect.h - 2 * radius}, color);
    draw_corner(*this, {rect.x + radius, rect.y + radius}, radius, 0, color, false, thickness);
    draw_corner(*this, {rect.x + rect.w - radius, rect.y + radius}, radius, 1, color, false, thickness);
    draw_corner(*this, {rect.x + radius, rect.y + rect.h - radius}, radius, 2, color, false, thickness);
    draw_corner(*this, {rect.x + rect.w - radius, rect.y + rect.h - radius}, radius, 3, color, false, thickness);
}

}  // namespace Izo