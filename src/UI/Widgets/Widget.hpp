#pragma once

#include "Geometry/Primitives.hpp"
#include "Input/KeyCode.hpp"
#include "Motion/Animator.hpp"

namespace Izo {

class Painter;

enum class WidgetSizePolicy {
    Fixed = 0,
    MatchParent = -1,
    WrapContent = -2
};

class Widget {
public:
    Widget();
    virtual ~Widget() = default;

    virtual void draw_content(Painter& painter) = 0;
    virtual void draw_focus(Painter& painter); 

    virtual void update();

    virtual void layout() {};
    virtual void measure(int parent_w, int parent_h); 
    // NOTE: Deprecated, use local_bounds() instead
    virtual IntRect content_box() const { return {0, 0, m_bounds.w, m_bounds.h}; } 

    /* What's the difference between on_touch and on_touch_event?? */
    virtual bool on_touch(IntPoint point, bool down, bool captured = false); 
    virtual bool on_touch_event(IntPoint point, bool down) { return false; } 
    virtual bool on_scroll(int y) { return false; }
    virtual bool on_key(KeyCode key) { return false; }
    /* This should probably be a public function */
    virtual bool is_scrollable() const { return false; }

    virtual IntPoint content_scroll_offset() const { return {0, 0}; }

    /* Returns the widget bounds transformed with parent position and scroll position */
    const IntRect global_bounds() const;
    /* Returns the actual size and position of widget relative to it's parent */
    const IntRect& local_bounds() const { return m_bounds; }

    void draw(Painter& painter); 

    int width() const { return m_width; }
    int height() const { return m_height; }

    /* TODO: Find out the difference between width() and measured_width(), same for height*/
    int measured_width() const { return m_measured_size.w; }
    int measured_height() const { return m_measured_size.h; }

    void show();
    void hide();

    bool visible() const { return m_visible; }
    bool hovering() const;

    void set_padding(Padding padding);
    const Padding padding() const;

    void set_height(int h) { m_height = h; }
    void set_height(WidgetSizePolicy p) { m_height = (int)p; }

    bool focusable() const { return m_focusable; }
    bool focused() const { return m_focused; }
    void cancel_gesture() { m_gesture_cancelled = true; }

    void set_bounds(const IntRect& bounds) { m_bounds = bounds; }
    void set_width(WidgetSizePolicy p) { m_width = (int)p; }
    void set_width(int w) { m_width = w; }
    void set_focused(bool focused);
    void set_focusable(bool focusable) { m_focusable = focusable; }
    void set_padding_ltrb(int left, int top, int right, int bottom);
    void set_padding(int padding);
    void set_show_focus_indicator(bool show) { m_show_focus_indicator = show; }
    void set_parent(Widget* parent) { m_parent = parent; }
    Widget* parent() const { return m_parent; }

    void set_layout_index(int index) { m_layout_index = index; }
    int layout_index() const { return m_layout_index; }

protected:
    void handle_focus_logic(bool inside, bool down);
    void draw_focus_outline(Painter& painter);
    void draw_debug_info(Painter& painter);

    IntRect m_bounds;
    IntRect m_measured_size;

    int m_width = (int)WidgetSizePolicy::WrapContent;
    int m_height = (int)WidgetSizePolicy::WrapContent;
    int m_layout_index = -1;

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

} 