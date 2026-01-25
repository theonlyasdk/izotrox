// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "Painter.hpp"
#include "Animator.hpp"
#include "Geometry/Rect.hpp"
#include "Input/KeyCode.hpp"

namespace Izo {

enum class WidgetSizePolicy {
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
    virtual void draw_focus(Painter& painter); // Called in separate pass for overlays

    virtual void update();
    
    // Layout
    virtual void layout() {};
    virtual void measure(int parent_w, int parent_h); // Sets m_measured_size
    virtual IntRect content_box() const { return {0, 0, m_bounds.w, m_bounds.h}; } // Default content is full size

    // Input
    virtual bool on_touch(IntPoint point, bool down, bool captured = false); // Template method
    virtual bool on_touch_event(IntPoint point, bool down) { return false; } // For subclass handling
    virtual bool on_scroll(int y) { return false; }
    virtual bool on_key(KeyCode key) { return false; }
    virtual bool is_scrollable() const { return false; }

    virtual IntPoint content_scroll_offset() const { return {0, 0}; }

    // Properties
    void set_bounds(const IntRect& bounds) { m_bounds = bounds; }
    IntRect bounds() const;
    
    void set_width(int w) { m_width = w; }
    void set_width(WidgetSizePolicy p) { m_width = (int)p; }
    int width() const { return m_width; }

    void set_height(int h) { m_height = h; }
    void set_height(WidgetSizePolicy p) { m_height = (int)p; }
    int height() const { return m_height; }
    
    int measured_width() const { return m_measured_size.w; }
    int measured_height() const { return m_measured_size.h; }

    void show();
    void hide();
    bool visible() const { return m_visible; }
    bool hovering() const;

    void set_focusable(bool focusable) { m_focusable = focusable; }
    bool focusable() const { return m_focusable; }
    bool focused() const { return m_focused; }
    void set_focused(bool focused);
    void cancel_gesture() { m_gesture_cancelled = true; }
    
    void set_padding(int l, int t, int r, int b) { 
        m_padding_left = l; m_padding_top = t; m_padding_right = r; m_padding_bottom = b; 
    }
    
    void set_show_focus_indicator(bool show) { m_show_focus_indicator = show; }
    
    void set_parent(Widget* parent) { m_parent = parent; }
    Widget* parent() const { return m_parent; }

protected:
    void handle_focus_logic(bool inside, bool down);
    void draw_focus_outline(Painter& painter);

    IntRect m_bounds;
    IntRect m_measured_size;
    
    int m_width = (int)WidgetSizePolicy::WrapContent;
    int m_height = (int)WidgetSizePolicy::WrapContent;
    
    int m_padding_left = 0;
    int m_padding_right = 0;
    int m_padding_top = 0;
    int m_padding_bottom = 0;

    bool m_prev_touch_down = false;
    bool m_touch_started_inside = false;
    bool m_gesture_cancelled = false;
    bool m_focusable = false;
    bool m_focused = false;
    bool m_visible = true;
    bool m_show_focus_indicator = true;
    Animator<float> m_focus_anim;
    Widget* m_parent = nullptr;
};

} // namespace Izo