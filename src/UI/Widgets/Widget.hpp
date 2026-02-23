#pragma once

#include <string>
#include "Geometry/Primitives.hpp"
#include "Graphics/Color.hpp"
#include "Input/KeyCode.hpp"
#include "Motion/Animator.hpp"

namespace Izo {

class Painter;
class Font;

enum class WidgetSizePolicy {
    Fixed = 0,
    MatchParent = -1,
    WrapContent = -2
};

class Widget {
public:
    Widget();
    virtual ~Widget();

    static void notify_theme_update_all();

    virtual void draw_content(Painter& painter) = 0;
    virtual void draw_focus(Painter& painter); 

    virtual void update();

    /* NOTE: Child classes must call invalidate_visual() in this
        method to re-render the widget */
    virtual void on_theme_update();

    virtual void layout() {};
    virtual void measure(int parent_w, int parent_h); 

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
    /* Returns the content box of the widget (IntRect with only size) */
    const IntRect content_box() const { return {0, 0, m_bounds.w, m_bounds.h }; }

    void draw(Painter& painter); 

    void set_font(Font* font);
    Font* font() const { return m_font; }

    int width() const { return m_width; }
    int height() const { return m_height; }

    /* TODO: Find out the difference between width() and measured_width(), same for height*/
    int measured_width() const { return m_measured_size.w; }
    int measured_height() const { return m_measured_size.h; }

    void show();
    void hide();

    bool visible() const { return m_visible; }
    bool hovering() const;

    void set_padding(Padding padding) { m_padding = padding; };
    void set_padding_ltrb(int left, int top, int right, int bottom);
    const Padding padding() const { return m_padding; }

    void set_height(int h);
    void set_height(WidgetSizePolicy p);

    bool focusable() const { return m_focusable; }
    bool focused() const { return m_focused; }
    void cancel_gesture() { m_gesture_cancelled = true; }

    void set_bounds(const IntRect& bounds);
    void set_width(WidgetSizePolicy p);
    void set_width(int w);
    void set_focused(bool focused);
    void set_focusable(bool focusable);
    void set_show_focus_indicator(bool show);
    void set_parent(Widget* parent);
    Widget* parent() const { return m_parent; }

    void set_layout_index(int index);
    int layout_index() const { return m_layout_index; }

    void invalidate_visual();
    void invalidate_layout();
    bool layout_dirty() const { return m_layout_dirty; }

    virtual bool subtree_layout_dirty() const { return m_layout_dirty; }
    virtual void clear_layout_dirty_subtree() { m_layout_dirty = false; }
    virtual bool has_running_animations() const;

    const std::string widget_type() const { return m_widget_type; };
protected:
    void handle_focus_logic(bool inside, bool down);
    void draw_focus_outline(Painter& painter);
    void draw_debug_info(Painter& painter);
    void set_widget_type(const std::string type) { m_widget_type = type; };
    void finalize_widget_construction() { on_theme_update(); }

    std::string m_widget_type;

    IntRect m_bounds;
    IntRect m_measured_size;
    Padding m_padding {2};

    int m_width = static_cast<int>(WidgetSizePolicy::WrapContent);
    int m_height = static_cast<int>(WidgetSizePolicy::WrapContent);
    int m_layout_index = -1;

    bool m_prev_touch_down = false;
    bool m_touch_started_inside = false;
    bool m_gesture_cancelled = false;
    bool m_focusable = false;
    bool m_focused = false;
    bool m_visible = true;
    bool m_show_focus_indicator = true;
    Animator<float> m_focus_anim;
    Widget* m_parent = nullptr;
    Font* m_font = nullptr;
    int m_focus_outline_thickness = 12;
    int m_focus_roundness = 6;
    Color m_focus_color = Color(0, 0, 255);
    int m_focus_anim_duration = 300;
    bool m_layout_dirty = true;
};

} 
