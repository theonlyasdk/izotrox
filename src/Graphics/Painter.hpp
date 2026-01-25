

#pragma once
#include "Canvas.hpp"
#include "Color.hpp"
#include "Geometry/Rect.hpp"
#include <algorithm>
#include <vector>

namespace Izo {

class Painter {
public:
    Painter(Canvas& canvas);

    void set_canvas(Canvas& canvas);
    void push_clip(const IntRect& rect);
    void pop_clip();

    void push_translate(IntPoint offset);
    void pop_translate();

    void draw_pixel(IntPoint point, Color color);
    void fill_rect(const IntRect& rect, Color color);

    void clear_rect(const IntRect& rect, Color color);

    void draw_rect(const IntRect& rect, Color color);
    void draw_line(IntPoint p1, IntPoint p2, Color color);

    void fill_rounded_rect(const IntRect& rect, int radius, Color color);
    void draw_rounded_rect(const IntRect& rect, int radius, Color color, int thickness = 1);

    Canvas& canvas() { return *m_canvas; }

private:
    Canvas* m_canvas;
    IntRect m_clip_rect;
    std::vector<IntRect> m_clip_stack;

    int m_tx = 0;
    int m_ty = 0;
    struct Translation { int x; int y; };
    std::vector<Translation> m_translate_stack;
};

} 