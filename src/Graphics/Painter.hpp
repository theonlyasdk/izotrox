
#pragma once
#include "Canvas.hpp"
#include "Color.hpp"
#include "Geometry/Primitives.hpp"
#include <algorithm>
#include <vector>

namespace Izo
{

class Painter
{
    enum Corner
    {
        TopLeft = 1 << 0,
        TopRight = 1 << 1,
        BottomRight = 1 << 2,
        BottomLeft = 1 << 3,
        AllCorners = TopLeft | TopRight | BottomRight | BottomLeft
    };

  public:
    Painter(Canvas &canvas);
    void set_canvas(Canvas &canvas);
    void push_clip(const IntRect &rect);
    void push_rounded_clip(const IntRect &rect, int radius);
    void pop_clip();
    void set_global_alpha(float alpha)
    {
        m_global_alpha = std::clamp(alpha, 0.0f, 1.0f);
    }
    float global_alpha() const
    {
        return m_global_alpha;
    }
    void push_translate(IntPoint offset);
    void pop_translate();
    void draw_pixel(IntPoint point, Color color);
    void fill_rect(const IntRect &rect, Color color);
    void clear_rect(const IntRect &rect, Color color);
    void draw_rect(const IntRect &rect, Color color);
    void draw_line(IntPoint p1, IntPoint p2, Color color);
    void fill_rounded_rect(const IntRect &rect, int radius, Color color, int corners = AllCorners);
    void draw_rounded_rect(const IntRect &rect, int radius, Color color, int thickness = 1);
    Canvas &canvas()
    {
        return *m_canvas;
    }

  private:
    Canvas *m_canvas;
    int m_tx = 0;
    int m_ty = 0;
    struct Translation
    {
        int x;
        int y;
    };
    std::vector<Translation> m_translate_stack;
    struct ClipRect
    {
        IntRect rect;
        int radius;
    };
    ClipRect m_current_clip;
    std::vector<ClipRect> m_clip_stack;
    float m_global_alpha = 1.0f;
};

} // namespace Izo