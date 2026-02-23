#include "Graphics/Painter.hpp"

#include "Graphics/Canvas.hpp"
#include "Graphics/Color.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace Izo {

static inline uint32_t blend_argb_over(uint32_t src, uint32_t dst, uint32_t alpha) {
    const uint32_t inv_alpha = 256U - alpha;
    const uint32_t src_rb = src & 0x00FF00FFU;
    const uint32_t src_g = src & 0x0000FF00U;
    const uint32_t dst_rb = dst & 0x00FF00FFU;
    const uint32_t dst_g = dst & 0x0000FF00U;

    const uint32_t out_rb = ((src_rb * alpha + dst_rb * inv_alpha) >> 8) & 0x00FF00FFU;
    const uint32_t out_g = ((src_g * alpha + dst_g * inv_alpha) >> 8) & 0x0000FF00U;
    return 0xFF000000U | out_rb | out_g;
}

static inline bool point_inside_rounded_clip(const IntRect& clip, int radius, int x, int y) {
    if (radius <= 0) {
        return true;
    }

    const int max_radius = std::min(clip.w, clip.h) / 2;
    const int r = std::min(radius, max_radius);
    if (r <= 0) {
        return true;
    }

    const int left = clip.x;
    const int top = clip.y;
    const int right = clip.right();
    const int bottom = clip.bottom();

    if (x >= left + r && x < right - r) {
        return true;
    }
    if (y >= top + r && y < bottom - r) {
        return true;
    }

    int cx = 0;
    int cy = 0;
    if (x < left + r && y < top + r) {
        cx = left + r;
        cy = top + r;
    } else if (x >= right - r && y < top + r) {
        cx = right - r;
        cy = top + r;
    } else if (x < left + r && y >= bottom - r) {
        cx = left + r;
        cy = bottom - r;
    } else {
        cx = right - r;
        cy = bottom - r;
    }

    const int dx = x - cx;
    const int dy = y - cy;
    return dx * dx + dy * dy <= r * r;
}

static inline bool point_inside_rounded_rect_local(int x, int y, int width, int height, int radius) {
    if (radius <= 0) {
        return true;
    }

    const int r = std::min(radius, std::min(width, height) / 2);
    if (r <= 0) {
        return true;
    }

    if (x >= r && x < width - r) {
        return true;
    }
    if (y >= r && y < height - r) {
        return true;
    }

    const int cx = (x < r) ? (r - 1) : (width - r);
    const int cy = (y < r) ? (r - 1) : (height - r);
    const int dx = x - cx;
    const int dy = y - cy;
    return dx * dx + dy * dy <= r * r;
}

static void blur_alpha_mask(std::vector<uint8_t>& alpha, int width, int height, int radius) {
    if (radius <= 0 || width <= 0 || height <= 0) {
        return;
    }

    const int kernel = radius * 2 + 1;
    std::vector<uint8_t> temp(static_cast<size_t>(width) * static_cast<size_t>(height), 0);

    for (int y = 0; y < height; ++y) {
        const size_t row_base = static_cast<size_t>(y) * static_cast<size_t>(width);
        int sum = 0;

        for (int k = -radius; k <= radius; ++k) {
            int sx = std::clamp(k, 0, width - 1);
            sum += alpha[row_base + static_cast<size_t>(sx)];
        }

        for (int x = 0; x < width; ++x) {
            temp[row_base + static_cast<size_t>(x)] = static_cast<uint8_t>(sum / kernel);

            int remove_x = std::clamp(x - radius, 0, width - 1);
            int add_x = std::clamp(x + radius + 1, 0, width - 1);
            sum += static_cast<int>(alpha[row_base + static_cast<size_t>(add_x)]) -
                   static_cast<int>(alpha[row_base + static_cast<size_t>(remove_x)]);
        }
    }

    for (int x = 0; x < width; ++x) {
        int sum = 0;
        for (int k = -radius; k <= radius; ++k) {
            int sy = std::clamp(k, 0, height - 1);
            sum += temp[static_cast<size_t>(sy) * static_cast<size_t>(width) + static_cast<size_t>(x)];
        }

        for (int y = 0; y < height; ++y) {
            alpha[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] =
                static_cast<uint8_t>(sum / kernel);

            int remove_y = std::clamp(y - radius, 0, height - 1);
            int add_y = std::clamp(y + radius + 1, 0, height - 1);
            sum += static_cast<int>(temp[static_cast<size_t>(add_y) * static_cast<size_t>(width) + static_cast<size_t>(x)]) -
                   static_cast<int>(temp[static_cast<size_t>(remove_y) * static_cast<size_t>(width) + static_cast<size_t>(x)]);
        }
    }
}

Painter::Painter(std::unique_ptr<Canvas> canvas) : m_canvas(std::move(canvas)) {
    m_current_clip = {{0, 0, m_canvas->width(), m_canvas->height()}, 0};
}

void Painter::set_global_alpha(float alpha) {
    m_global_alpha = std::clamp(alpha, 0.0f, 1.0f);
}

void Painter::reset_clips_and_transform() {
    m_current_clip = {{0, 0, m_canvas->width(), m_canvas->height()}, 0};
    m_clip_stack.clear();
    m_translate_stack.clear();
    m_translation = {0, 0};
}

void Painter::set_canvas(std::unique_ptr<Canvas> canvas) {
    m_canvas = std::move(canvas);
    reset_clips_and_transform();
}

void Painter::push_rounded_clip(const IntRect& rect, int radius) {
    m_clip_stack.push_back(m_current_clip);
    const IntRect translated = apply_translate_to_rect(rect);
    m_current_clip = {translated.intersection(m_current_clip.rect), radius};
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
    m_translate_stack.push_back(m_translation);
    m_translation += offset;
}

void Painter::pop_translate() {
    if (!m_translate_stack.empty()) {
        m_translation = m_translate_stack.back();
        m_translate_stack.pop_back();
    }
}

const Painter::ShadowLayer& Painter::get_or_build_shadow_layer(int rect_w, int rect_h, int blur_radius, int roundness) {
    rect_w = std::max(1, rect_w);
    rect_h = std::max(1, rect_h);
    blur_radius = std::max(0, blur_radius);
    roundness = std::clamp(roundness, 0, std::min(rect_w, rect_h) / 2);

    if (m_shadow_layer_cache.rect_w == rect_w &&
        m_shadow_layer_cache.rect_h == rect_h &&
        m_shadow_layer_cache.blur_radius == blur_radius &&
        m_shadow_layer_cache.roundness == roundness &&
        !m_shadow_layer_cache.alpha.empty()) {
        return m_shadow_layer_cache;
    }

    ShadowLayer layer;
    layer.rect_w = rect_w;
    layer.rect_h = rect_h;
    layer.blur_radius = blur_radius;
    layer.roundness = roundness;
    layer.layer_w = rect_w + blur_radius * 2;
    layer.layer_h = rect_h + blur_radius * 2;
    layer.alpha.assign(static_cast<size_t>(layer.layer_w) * static_cast<size_t>(layer.layer_h), 0);

    const int shape_x = blur_radius;
    const int shape_y = blur_radius;
    for (int y = 0; y < rect_h; ++y) {
        const int ly = shape_y + y;
        uint8_t* row = layer.alpha.data() + static_cast<size_t>(ly) * static_cast<size_t>(layer.layer_w);

        if (roundness <= 0) {
            std::fill_n(row + shape_x, rect_w, static_cast<uint8_t>(255));
            continue;
        }

        for (int x = 0; x < rect_w; ++x) {
            if (point_inside_rounded_rect_local(x, y, rect_w, rect_h, roundness)) {
                row[shape_x + x] = 255;
            }
        }
    }

    blur_alpha_mask(layer.alpha, layer.layer_w, layer.layer_h, blur_radius);
    m_shadow_layer_cache = std::move(layer);
    return m_shadow_layer_cache;
}

void Painter::draw_pixel(IntPoint point, Color color) {
    if (m_global_alpha <= 0.0f) {
        return;
    }

    const int x = point.x + m_translation.x;
    const int y = point.y + m_translation.y;

    const int canvas_w = m_canvas->width();
    const int canvas_h = m_canvas->height();
    if (static_cast<unsigned>(x) >= static_cast<unsigned>(canvas_w) ||
        static_cast<unsigned>(y) >= static_cast<unsigned>(canvas_h)) {
        return;
    }

    const IntRect& clip = m_current_clip.rect;
    if (x < clip.x || x >= clip.right() || y < clip.y || y >= clip.bottom()) {
        return;
    }

    if (!point_inside_rounded_clip(clip, m_current_clip.radius, x, y)) {
        return;
    }

    const uint32_t alpha = static_cast<uint32_t>(color.a * m_global_alpha);
    if (alpha == 0) {
        return;
    }

    uint32_t* const pixels = m_canvas->pixels();
    uint32_t& dst = pixels[y * canvas_w + x];
    const uint32_t src = color.as_argb();

    if (alpha >= 255U) {
        dst = src;
    } else {
        dst = blend_argb_over(src, dst, alpha);
    }
}

void Painter::fill_rect(const IntRect& rect, Color color) {
    if (rect.w <= 0 || rect.h <= 0 || m_global_alpha <= 0.0f) {
        return;
    }

    IntRect dest = apply_translate_to_rect(rect).intersection(m_current_clip.rect);
    if (dest.w <= 0 || dest.h <= 0) {
        return;
    }

    Color final_color = color;
    final_color.a = static_cast<uint8_t>(color.a * m_global_alpha);
    const uint32_t alpha = final_color.a;
    if (alpha == 0) {
        return;
    }

    const uint32_t src = final_color.as_argb();
    const int stride = m_canvas->width();
    uint32_t* const pixels = m_canvas->pixels();

    if (m_current_clip.radius > 0) {
        const IntRect& clip = m_current_clip.rect;
        const int radius = m_current_clip.radius;

        for (int y = dest.y; y < dest.bottom(); ++y) {
            uint32_t* row = pixels + y * stride;
            for (int x = dest.x; x < dest.right(); ++x) {
                if (!point_inside_rounded_clip(clip, radius, x, y)) {
                    continue;
                }

                uint32_t& dst = row[x];
                if (alpha == 255U) {
                    dst = src;
                } else {
                    dst = blend_argb_over(src, dst, alpha);
                }
            }
        }
        return;
    }

    if (alpha == 255U) {
        for (int y = 0; y < dest.h; ++y) {
            uint32_t* row = pixels + (dest.y + y) * stride + dest.x;
            std::fill_n(row, dest.w, src);
        }
        return;
    }

    for (int y = 0; y < dest.h; ++y) {
        uint32_t* row = pixels + (dest.y + y) * stride + dest.x;
        int x = 0;

        for (; x + 3 < dest.w; x += 4) {
            row[x] = blend_argb_over(src, row[x], alpha);
            row[x + 1] = blend_argb_over(src, row[x + 1], alpha);
            row[x + 2] = blend_argb_over(src, row[x + 2], alpha);
            row[x + 3] = blend_argb_over(src, row[x + 3], alpha);
        }
        for (; x < dest.w; ++x) {
            row[x] = blend_argb_over(src, row[x], alpha);
        }
    }
}

void Painter::clear_rect(const IntRect& rect, Color color) {
    fill_rect(rect, color);
}

void Painter::outline_rect(const IntRect& rect, Color color) {
    if (rect.w <= 0 || rect.h <= 0) {
        return;
    }

    fill_rect({rect.x, rect.y, rect.w, 1}, color);
    fill_rect({rect.x, rect.y + rect.h - 1, rect.w, 1}, color);
    fill_rect({rect.x, rect.y, 1, rect.h}, color);
    fill_rect({rect.x + rect.w - 1, rect.y, 1, rect.h}, color);
}

void Painter::draw_line(IntPoint p1, IntPoint p2, Color color) {
    int x1 = p1.x;
    int y1 = p1.y;
    const int x2 = p2.x;
    const int y2 = p2.y;

    const int dx = std::abs(x2 - x1);
    const int dy = -std::abs(y2 - y1);
    const int sx = x1 < x2 ? 1 : -1;
    const int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        draw_pixel({x1, y1}, color);
        if (x1 == x2 && y1 == y2) {
            break;
        }

        const int e2 = err * 2;
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

void Painter::drop_shadow_rect(const IntRect& rect, int blur_radius, Color color, int roundness, IntPoint offset) {
    if (rect.w <= 0 || rect.h <= 0 || color.a == 0 || m_global_alpha <= 0.0f) {
        return;
    }

    blur_radius = std::max(0, blur_radius);
    const ShadowLayer& layer = get_or_build_shadow_layer(rect.w, rect.h, blur_radius, roundness);
    if (layer.alpha.empty()) {
        return;
    }

    IntRect target = {
        rect.x + offset.x - blur_radius + m_translation.x,
        rect.y + offset.y - blur_radius + m_translation.y,
        layer.layer_w,
        layer.layer_h,
    };

    const IntRect canvas_rect = {0, 0, m_canvas->width(), m_canvas->height()};
    IntRect clipped = target.intersection(m_current_clip.rect).intersection(canvas_rect);
    if (clipped.w <= 0 || clipped.h <= 0) {
        return;
    }

    const int src_x0 = clipped.x - target.x;
    const int src_y0 = clipped.y - target.y;
    const uint32_t source = Color(color.r, color.g, color.b, 255).as_argb();
    const uint32_t global_alpha = static_cast<uint32_t>(std::clamp(m_global_alpha, 0.0f, 1.0f) * 255.0f);

    uint32_t* pixels = m_canvas->pixels();
    const int stride = m_canvas->width();

    const IntRect& clip = m_current_clip.rect;
    const int clip_radius = m_current_clip.radius;
    const bool clip_is_rounded = clip_radius > 0;

    for (int y = 0; y < clipped.h; ++y) {
        const int py = clipped.y + y;
        const uint8_t* src_row =
            layer.alpha.data() + static_cast<size_t>(src_y0 + y) * static_cast<size_t>(layer.layer_w) + static_cast<size_t>(src_x0);
        uint32_t* dst_row = pixels + py * stride + clipped.x;

        for (int x = 0; x < clipped.w; ++x) {
            const int px = clipped.x + x;
            if (clip_is_rounded && !point_inside_rounded_clip(clip, clip_radius, px, py)) {
                continue;
            }

            uint32_t alpha = src_row[x];
            if (alpha == 0) {
                continue;
            }

            alpha = (alpha * color.a + 127U) / 255U;
            alpha = (alpha * global_alpha + 127U) / 255U;
            if (alpha == 0) {
                continue;
            }

            uint32_t& dst = dst_row[x];
            if (alpha >= 255U) {
                dst = source;
            } else {
                dst = blend_argb_over(source, dst, alpha);
            }
        }
    }
}

static void draw_corner(Painter& painter, IntPoint center, int radius, int quad,
                        Color color, bool filled, int thickness = 1) {
    const int cx = center.x;
    const int cy = center.y;
    const float r = static_cast<float>(radius);

    for (int y = 0; y < radius; ++y) {
        for (int x = 0; x < radius; ++x) {
            const float dx = static_cast<float>(x) + 0.5f;
            const float dy = static_cast<float>(y) + 0.5f;
            const float dist = std::sqrt(dx * dx + dy * dy);

            float alpha = 0.0f;
            if (filled) {
                if (dist <= r - 0.5f) {
                    alpha = 1.0f;
                } else if (dist < r + 0.5f) {
                    alpha = 1.0f - (dist - (r - 0.5f));
                }
            } else {
                const float t = static_cast<float>(thickness);
                const float half_t = t / 2.0f;
                const float center_r = r - half_t;
                const float d = std::abs(dist - center_r);
                if (d < half_t - 0.5f) {
                    alpha = 1.0f;
                } else if (d < half_t + 0.5f) {
                    alpha = 1.0f - (d - (half_t - 0.5f));
                }
            }

            if (alpha <= 0.0f) {
                continue;
            }

            Color c = color;
            c.a = static_cast<uint8_t>(c.a * std::clamp(alpha, 0.0f, 1.0f));

            const int px = (quad == 0 || quad == 2) ? (cx - 1 - x) : (cx + x);
            const int py = (quad == 0 || quad == 1) ? (cy - 1 - y) : (cy + y);
            painter.draw_pixel({px, py}, c);
        }
    }
}

void Painter::fill_rounded_rect(const IntRect& rect, int radius, Color color, int corners) {
    if (rect.w <= 0 || rect.h <= 0) {
        return;
    }

    if (radius <= 0) {
        fill_rect(rect, color);
        return;
    }

    radius = std::min(radius, std::min(rect.w, rect.h) / 2);

    fill_rect({rect.x + radius, rect.y, rect.w - 2 * radius, rect.h}, color);
    fill_rect({rect.x, rect.y + radius, radius, rect.h - 2 * radius}, color);
    fill_rect({rect.x + rect.w - radius, rect.y + radius, radius, rect.h - 2 * radius}, color);

    if (corners & Corner::TopLeft) {
        draw_corner(*this, {rect.x + radius, rect.y + radius}, radius, 0, color, true);
    } else {
        fill_rect({rect.x, rect.y, radius, radius}, color);
    }

    if (corners & Corner::TopRight) {
        draw_corner(*this, {rect.x + rect.w - radius, rect.y + radius}, radius, 1, color, true);
    } else {
        fill_rect({rect.x + rect.w - radius, rect.y, radius, radius}, color);
    }

    if (corners & Corner::BottomLeft) {
        draw_corner(*this, {rect.x + radius, rect.y + rect.h - radius}, radius, 2, color, true);
    } else {
        fill_rect({rect.x, rect.y + rect.h - radius, radius, radius}, color);
    }

    if (corners & Corner::BottomRight) {
        draw_corner(*this, {rect.x + rect.w - radius, rect.y + rect.h - radius}, radius, 3, color, true);
    } else {
        fill_rect({rect.x + rect.w - radius, rect.y + rect.h - radius, radius, radius}, color);
    }
}

void Painter::draw_rounded_rect(const IntRect& rect, int radius, Color color, int thickness) {
    if (rect.w <= 0 || rect.h <= 0 || thickness <= 0) {
        return;
    }

    if (radius <= 0) {
        fill_rect({rect.x, rect.y, rect.w, thickness}, color);
        fill_rect({rect.x, rect.y + rect.h - thickness, rect.w, thickness}, color);
        fill_rect({rect.x, rect.y + thickness, thickness, rect.h - 2 * thickness}, color);
        fill_rect({rect.x + rect.w - thickness, rect.y + thickness, thickness, rect.h - 2 * thickness}, color);
        return;
    }

    radius = std::min(radius, std::min(rect.w, rect.h) / 2);

    fill_rect({rect.x + radius, rect.y, rect.w - 2 * radius, thickness}, color);
    fill_rect({rect.x + radius, rect.y + rect.h - thickness, rect.w - 2 * radius, thickness}, color);
    fill_rect({rect.x, rect.y + radius, thickness, rect.h - 2 * radius}, color);
    fill_rect({rect.x + rect.w - thickness, rect.y + radius, thickness, rect.h - 2 * radius}, color);

    draw_corner(*this, {rect.x + radius, rect.y + radius}, radius, 0, color, false, thickness);
    draw_corner(*this, {rect.x + rect.w - radius, rect.y + radius}, radius, 1, color, false, thickness);
    draw_corner(*this, {rect.x + radius, rect.y + rect.h - radius}, radius, 2, color, false, thickness);
    draw_corner(*this, {rect.x + rect.w - radius, rect.y + rect.h - radius}, radius, 3, color, false, thickness);
}

void Painter::draw_blur_rect(const IntRect& rect, int blur_level) {
    if (blur_level <= 0) {
        return;
    }

    IntRect area = apply_translate_to_rect(rect).intersection(m_current_clip.rect);
    area = area.intersection({0, 0, m_canvas->width(), m_canvas->height()});
    if (area.w <= 0 || area.h <= 0) {
        return;
    }

    const int width = area.w;
    const int height = area.h;
    const int radius = std::min(blur_level, std::max(width, height) - 1);
    if (radius <= 0) {
        return;
    }

    const int kernel_size = radius * 2 + 1;
    const int half = kernel_size / 2;
    const int stride = m_canvas->width();
    uint32_t* canvas_pixels = m_canvas->pixels();

    const size_t pixel_count = static_cast<size_t>(width) * static_cast<size_t>(height);
    static thread_local std::vector<uint32_t> src;
    static thread_local std::vector<uint32_t> temp;
    src.resize(pixel_count);
    temp.resize(pixel_count);

    for (int y = 0; y < height; ++y) {
        uint32_t* src_row = canvas_pixels + (area.y + y) * stride + area.x;
        std::copy_n(src_row, width, src.data() + static_cast<size_t>(y) * width);
    }

    for (int y = 0; y < height; ++y) {
        const size_t row_base = static_cast<size_t>(y) * width;
        int s0 = 0;
        int s1 = 0;
        int s2 = 0;
        int sa = 0;

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

    for (int x = 0; x < width; ++x) {
        int s0 = 0;
        int s1 = 0;
        int s2 = 0;
        int sa = 0;

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
