// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "Painter.hpp"
#include "Animator.hpp"
#include "Geometry/Rect.hpp"
#include "Input/KeyCode.hpp"

namespace Izo {

enum SizePolicy {
    Fixed = 0,
    MatchParent = -1,
    WrapContent = -2
};

class Widget {
public:
    Widget();
    virtual ~Widget() = default;

    // Drawing
    void draw(Painter& painter); // Template method
    virtual void draw_content(Painter& painter) = 0;

    virtual void update();
    
    // Layout
    virtual void layout() {};
    virtual void measure(int parent_w, int parent_h); // Sets m_measured_size
    virtual IntRect content_box() const { return {0, 0, m_bounds.w, m_bounds.h}; } // Default content is full size

    // Input
    virtual bool on_touch(int tx, int ty, bool down, bool captured = false); // Template method
    virtual bool on_touch_event(int local_x, int local_y, bool down) { return false; } // For subclass handling
    virtual bool on_key(KeyCode key) { return false; }

    // Properties
    void set_bounds(const IntRect& bounds) { m_bounds = bounds; }
    const IntRect& bounds() const { return m_bounds; }
    
    void set_width(int w) { m_width = w; }
    void set_height(int h) { m_height = h; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    
    int measured_width() const { return m_measured_size.w; }
    int measured_height() const { return m_measured_size.h; }

    void show();
    void hide();
    bool visible() const { return m_visible; }

    void set_focusable(bool focusable) { m_is_focusable = focusable; }
    bool focusable() const { return m_is_focusable; }
    bool focused() const { return m_is_focused; }
    void set_focused(bool focused);
    
    void set_show_focus_indicator(bool show) { m_show_focus_indicator = show; }

    virtual void invalidate();
    static bool dirty();
    static void clear_dirty();

protected:
    void handle_focus_logic(bool inside, bool down);
    void draw_focus_outline(Painter& painter);

    IntRect m_bounds;
    IntRect m_measured_size;
    
    int m_width = WrapContent;
    int m_height = WrapContent;
    
    int m_padding_left = 0;
    int m_padding_right = 0;
    int m_padding_top = 0;
    int m_padding_bottom = 0;

    Animator<float> m_focus_anim;
    bool m_prev_touch_down = false;
    bool m_is_focusable = false;
    bool m_is_focused = false;
    bool m_visible = true;
    bool m_show_focus_indicator = true;

    static bool s_global_dirty;
};

} // namespace Izo