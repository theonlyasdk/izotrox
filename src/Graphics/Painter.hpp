// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "Canvas.hpp"
#include "Color.hpp"
#include "Core/Rect.hpp"
#include <algorithm>

namespace Izo {

class Painter {
public:
    Painter(Canvas& canvas);

    void set_clip(int x, int y, int w, int h);
    void reset_clip();

    void draw_pixel(int x, int y, Color color);
    void fill_rect(int x, int y, int w, int h, Color color);
    void draw_rect(int x, int y, int w, int h, Color color);
    void draw_line(int x1, int y1, int x2, int y2, Color color);
    
    void fill_rounded_rect(int x, int y, int w, int h, int radius, Color color);
    void draw_rounded_rect(int x, int y, int w, int h, int radius, Color color, int thickness = 1);

    Canvas& canvas() { return m_canvas_ref; }

private:
    Canvas& m_canvas_ref;
    IntRect m_clip_rect;
};

} // namespace Izo
