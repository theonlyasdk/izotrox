#pragma once
#include "Painter.hpp"

namespace Izo {

class Widget {
public:
    Widget() : x(0), y(0), w(0), h(0) {}
    virtual ~Widget() = default;

    virtual void draw(Painter& painter) = 0;
    virtual void update() {}
    virtual void measure(int& measured_w, int& measured_h) { measured_w = w; measured_h = h; }
    
    // Input
    virtual bool on_touch(int tx, int ty, bool down) { return false; }
    virtual bool on_key(int key) { return false; }

    virtual void set_pos(int x, int y) { this->x = x; this->y = y; }
    virtual void set_size(int w, int h) { this->w = w; this->h = h; }
    
    // Global invalidation flag
    static void invalidate() { global_dirty_ = true; }
    static bool is_dirty() { return global_dirty_; }
    static void clear_dirty() { global_dirty_ = false; }

    int x, y, w, h;
    bool is_focused = false;

private:
    static bool global_dirty_;
};

} // namespace Izo