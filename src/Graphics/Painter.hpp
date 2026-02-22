#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "Geometry/Primitives.hpp"

namespace Izo {

class Canvas;
class Color;

class Painter {
   public:
    enum Corner {
        TopLeft = 1 << 0,
        TopRight = 1 << 1,
        BottomRight = 1 << 2,
        BottomLeft = 1 << 3,
        AllCorners = TopLeft | TopRight | BottomRight | BottomLeft
    };

    Painter(std::unique_ptr<Canvas> canvas);
    void set_canvas(std::unique_ptr<Canvas> canvas);
    void push_clip(const IntRect& rect);
    void push_rounded_clip(const IntRect& rect, int radius);
    void pop_clip();
    void set_global_alpha(float alpha);
    void push_translate(IntPoint offset);
    void pop_translate();
    void draw_pixel(IntPoint point, Color color);
    void fill_rect(const IntRect& rect, Color color);
    void clear_rect(const IntRect& rect, Color color);
    void outline_rect(const IntRect& rect, Color color);
    void draw_line(IntPoint p1, IntPoint p2, Color color);
    void fill_rounded_rect(const IntRect& rect, int radius, Color color, int corners = AllCorners);
    void draw_rounded_rect(const IntRect& rect, int radius, Color color, int thickness = 1);
    void draw_drop_shadow_rect(const IntRect& rect, int blur_radius, Color color, int roundness = 0, IntPoint offset = {0, 0});
    void reset_clips_and_transform();
    void draw_blur_rect(const IntRect& rect, int blur_level);

    float global_alpha() const { return m_global_alpha; }
    Canvas* canvas() { return m_canvas.get(); }

   private:
    IntRect apply_translate_to_rect(const IntRect& rect) const {
        return IntRect{
            rect.x + m_translation.x,
            rect.y + m_translation.y,
            rect.w,
            rect.h,
        };
    }

    std::unique_ptr<Canvas> m_canvas;
    IntPoint m_translation{0, 0};
    std::vector<IntPoint> m_translate_stack;

    struct ClipRect {
        IntRect rect;
        int radius = 0;
    };

    struct ShadowLayer {
        int rect_w = 0;
        int rect_h = 0;
        int layer_w = 0;
        int layer_h = 0;
        int blur_radius = 0;
        int roundness = 0;
        std::vector<uint8_t> alpha;
    };

    const ShadowLayer& get_or_build_shadow_layer(int rect_w, int rect_h, int blur_radius, int roundness);

    ClipRect m_current_clip;
    std::vector<ClipRect> m_clip_stack;
    ShadowLayer m_shadow_layer_cache;
    float m_global_alpha = 1.0f;
};

}  // namespace Izo
