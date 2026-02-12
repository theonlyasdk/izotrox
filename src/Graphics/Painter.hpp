#pragma once

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
    void reset_clips_and_transform();

    float global_alpha() const { return m_global_alpha; }
    Canvas* canvas() { return m_canvas.get(); }

   private:
    IntRect apply_translate_to_rect(const IntRect& rect);

    std::unique_ptr<Canvas> m_canvas;
    int m_translate_x = 0;
    int m_translate_y = 0;
    struct Translation {
        int x;
        int y;
    };
    std::vector<Translation> m_translate_stack;
    struct ClipRect {
        IntRect rect;
        int radius;
    };
    ClipRect m_current_clip;
    std::vector<ClipRect> m_clip_stack;
    float m_global_alpha = 1.0f;
};

}  // namespace Izo