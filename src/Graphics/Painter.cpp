#include "Graphics/Painter.hpp"
#include "Graphics/Color.hpp"
#include "Graphics/Canvas.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "Geometry/Primitives.hpp"

namespace Izo {

static inline uint32_t blend_argb_over(uint32_t src, uint32_t dst, uint32_t a) {
    const uint32_t inv_a = 256 - a;
    const uint32_t src_rb = src & 0x00FF00FF;
    const uint32_t src_g = src & 0x0000FF00;
    const uint32_t dst_rb = dst & 0x00FF00FF;
    const uint32_t dst_g = dst & 0x0000FF00;

    const uint32_t out_rb = ((src_rb * a + dst_rb * inv_a) >> 8) & 0x00FF00FF;
    const uint32_t out_g = ((src_g * a + dst_g * inv_a) >> 8) & 0x0000FF00;
    return 0xFF000000 | out_rb | out_g;
}

Painter::Painter(std::unique_ptr<Canvas> canvas) : m_canvas(std::move(canvas)) {
    m_current_clip = {{0, 0, m_canvas->width(), m_canvas->height()}, 0};
}

IntRect Painter::apply_translate_to_rect(const IntRect& rect) {
    return IntRect{rect.x + m_translate_x, rect.y + m_translate_y, rect.w, rect.h};
}

void Painter::set_global_alpha(float alpha)  {
    m_global_alpha = std::clamp(alpha, 0.0f, 1.0f);
}

void Painter::reset_clips_and_transform() {
    m_current_clip = {{0, 0, m_canvas->width(), m_canvas->height()}, 0};
    m_clip_stack.clear();
    m_translate_stack.clear();
    m_translate_x = 0;
    m_translate_y = 0;
}

void Painter::set_canvas(std::unique_ptr<Canvas> canvas) {
    m_canvas = std::move(canvas);
    reset_clips_and_transform();
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
    int dx = point.x + m_translate_x;
    int dy = point.y + m_translate_y;

    const int canvas_w = m_canvas->width();
    const int canvas_h = m_canvas->height();
    if ((unsigned)dx >= (unsigned)canvas_w || (unsigned)dy >= (unsigned)canvas_h) return;

    const IntRect& cr = m_current_clip.rect;
    if (dx < cr.x || dx >= cr.right() || dy < cr.y || dy >= cr.bottom()) return;

    if (m_current_clip.radius > 0) {
        int rx = cr.x;
        int ry = cr.y;
        int rw = cr.w;
        int rh = cr.h;
        int r = m_current_clip.radius;

        int cx = -1;
        int cy = -1;

        if (dx < rx + r && dy < ry + r) {
            cx = rx + r;
            cy = ry + r;
        } else if (dx >= rx + rw - r && dy < ry + r) {
            cx = rx + rw - r;
            cy = ry + r;
        } else if (dx < rx + r && dy >= ry + rh - r) {
            cx = rx + r;
            cy = ry + rh - r;
        } else if (dx >= rx + rw - r && dy >= ry + rh - r) {
            cx = rx + rw - r;
            cy = ry + rh - r;
        }

        if (cx >= 0) {
            const int ddx = dx - cx;
            const int ddy = dy - cy;
            if (ddx * ddx + ddy * ddy > r * r) return;
        }
    }

    const uint32_t final_a = static_cast<uint32_t>(color.a * m_global_alpha);
    if (final_a == 0) return;

    uint32_t* pixels = m_canvas->pixels();
    uint32_t& dst = pixels[dy * canvas_w + dx];
    if (final_a == 255) {
        dst = color.as_argb();
        return;
    }

    dst = blend_argb_over(color.as_argb(), dst, final_a);
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

    const int stride = m_canvas->width();
    uint32_t* pixels = m_canvas->pixels();
    const uint32_t c = final_color.as_argb();

    if (final_color.a == 255) {
        for (int iy = 0; iy < dest.h; ++iy) {
            uint32_t* row = pixels + (dest.y + iy) * stride + dest.x;
            std::fill_n(row, dest.w, c);
        }
    } else {
        const uint32_t a = final_color.a;

        for (int iy = 0; iy < dest.h; ++iy) {
            uint32_t* row = pixels + (dest.y + iy) * stride + dest.x;
            int ix = 0;

            for (; ix + 3 < dest.w; ix += 4) {
                row[ix] = blend_argb_over(c, row[ix], a);
                row[ix + 1] = blend_argb_over(c, row[ix + 1], a);
                row[ix + 2] = blend_argb_over(c, row[ix + 2], a);
                row[ix + 3] = blend_argb_over(c, row[ix + 3], a);
            }
            for (; ix < dest.w; ++ix) {
                row[ix] = blend_argb_over(c, row[ix], a);
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

void Painter::draw_blur_rect(const IntRect& rect, int blur_level)
{
    if (blur_level <= 0) return;

    IntRect area = apply_translate_to_rect(rect).intersection(m_current_clip.rect);
    area = area.intersection({0, 0, m_canvas->width(), m_canvas->height()});
    if (area.w <= 0 || area.h <= 0) return;

    const int width = area.w;
    const int height = area.h;
    const int radius = std::min(blur_level, std::max(width, height) - 1);
    if (radius <= 0) return;

    const int kernel_size = radius * 2 + 1;
    const int half = kernel_size / 2;
    const int stride = m_canvas->width();
    uint32_t* canvas_pixels = m_canvas->pixels();

    const size_t pixel_count = static_cast<size_t>(width) * static_cast<size_t>(height);
    static thread_local std::vector<uint32_t> src;
    static thread_local std::vector<uint32_t> temp;
    src.resize(pixel_count);
    temp.resize(pixel_count);

    // Copy the source region into contiguous memory.
    for (int y = 0; y < height; ++y) {
        uint32_t* src_row = canvas_pixels + (area.y + y) * stride + area.x;
        std::copy_n(src_row, width, src.data() + static_cast<size_t>(y) * width);
    }

    // Horizontal pass (sliding window).
    for (int y = 0; y < height; ++y) {
        const size_t row_base = static_cast<size_t>(y) * width;
        int s0 = 0, s1 = 0, s2 = 0, sa = 0;

        for (int k = -radius; k <= radius; ++k) {
            int sx = std::clamp(k, 0, width - 1);
            uint32_t p = src[row_base + sx];
            s0 += p & 0xFF;
            s1 += (p >> 8) & 0xFF;
            s2 += (p >> 16) & 0xFF;
            sa += (p >> 24) & 0xFF;
        }

        for (int x = 0; x < width; ++x) {
            temp[row_base + x] =
                (static_cast<uint32_t>((sa + half) / kernel_size) << 24) |
                (static_cast<uint32_t>((s2 + half) / kernel_size) << 16) |
                (static_cast<uint32_t>((s1 + half) / kernel_size) << 8) |
                static_cast<uint32_t>((s0 + half) / kernel_size);

            int remove_x = std::clamp(x - radius, 0, width - 1);
            int add_x = std::clamp(x + radius + 1, 0, width - 1);
            uint32_t rp = src[row_base + remove_x];
            uint32_t ap = src[row_base + add_x];

            s0 += static_cast<int>(ap & 0xFF) - static_cast<int>(rp & 0xFF);
            s1 += static_cast<int>((ap >> 8) & 0xFF) - static_cast<int>((rp >> 8) & 0xFF);
            s2 += static_cast<int>((ap >> 16) & 0xFF) - static_cast<int>((rp >> 16) & 0xFF);
            sa += static_cast<int>((ap >> 24) & 0xFF) - static_cast<int>((rp >> 24) & 0xFF);
        }
    }

    // Vertical pass (sliding window), write back directly.
    for (int x = 0; x < width; ++x) {
        int s0 = 0, s1 = 0, s2 = 0, sa = 0;

        for (int k = -radius; k <= radius; ++k) {
            int sy = std::clamp(k, 0, height - 1);
            uint32_t p = temp[static_cast<size_t>(sy) * width + x];
            s0 += p & 0xFF;
            s1 += (p >> 8) & 0xFF;
            s2 += (p >> 16) & 0xFF;
            sa += (p >> 24) & 0xFF;
        }

        for (int y = 0; y < height; ++y) {
            canvas_pixels[(area.y + y) * stride + (area.x + x)] =
                (static_cast<uint32_t>((sa + half) / kernel_size) << 24) |
                (static_cast<uint32_t>((s2 + half) / kernel_size) << 16) |
                (static_cast<uint32_t>((s1 + half) / kernel_size) << 8) |
                static_cast<uint32_t>((s0 + half) / kernel_size);

            int remove_y = std::clamp(y - radius, 0, height - 1);
            int add_y = std::clamp(y + radius + 1, 0, height - 1);
            uint32_t rp = temp[static_cast<size_t>(remove_y) * width + x];
            uint32_t ap = temp[static_cast<size_t>(add_y) * width + x];

            s0 += static_cast<int>(ap & 0xFF) - static_cast<int>(rp & 0xFF);
            s1 += static_cast<int>((ap >> 8) & 0xFF) - static_cast<int>((rp >> 8) & 0xFF);
            s2 += static_cast<int>((ap >> 16) & 0xFF) - static_cast<int>((rp >> 16) & 0xFF);
            sa += static_cast<int>((ap >> 24) & 0xFF) - static_cast<int>((rp >> 24) & 0xFF);
        }
    }
}

}  // namespace Izo
