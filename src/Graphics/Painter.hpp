// Mozilla Public License version 2.0. (c) theonlyasdk 2026

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
    void push_clip(int x, int y, int w, int h);
    void pop_clip();
    
    void push_translate(int x, int y);
    void pop_translate();

    void draw_pixel(int x, int y, Color color);
    void fill_rect(int x, int y, int w, int h, Color color);
    
    // Fills the rect with a color, replacing existing pixels (no alpha blending)
    void clear_rect(int x, int y, int w, int h, Color color);

    void draw_rect(int x, int y, int w, int h, Color color);
    void draw_line(int x1, int y1, int x2, int y2, Color color);
    
    void fill_rounded_rect(int x, int y, int w, int h, int radius, Color color);
    void draw_rounded_rect(int x, int y, int w, int h, int radius, Color color, int thickness = 1);

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

} // namespace Izo