#pragma once
#include "Canvas.hpp"
#include "Color.hpp"
#include <algorithm>

namespace Izo {

struct Rect {
    int x, y, w, h;
    
    int right() const { return x + w; }
    int bottom() const { return y + h; }
    
    bool intersects(const Rect& other) const {
        return x < other.right() && right() > other.x &&
               y < other.bottom() && bottom() > other.y;
    }

    Rect intersection(const Rect& other) const {
        int x1 = std::max(x, other.x);
        int y1 = std::max(y, other.y);
        int x2 = std::min(right(), other.right());
        int y2 = std::min(bottom(), other.bottom());
        
        if (x2 < x1 || y2 < y1) return {0,0,0,0};
        return {x1, y1, x2 - x1, y2 - y1};
    }
};

class Painter {
public:
    Painter(Canvas& canvas);

    void set_clip(int x, int y, int w, int h);
    void reset_clip();

    void draw_pixel(int x, int y, Color color);
    void fill_rect(int x, int y, int w, int h, Color color);
    void draw_rect(int x, int y, int w, int h, Color color);
    void draw_line(int x1, int y1, int x2, int y2, Color color);
    
    // TODO: Add more primitives (circle, rounded rect, etc.)

    Canvas& canvas() { return canvas_; }

private:
    Canvas& canvas_;
    Rect clip_rect_;
};

} // namespace Izo
