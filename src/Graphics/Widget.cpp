

#include "Widget.hpp"
#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "Input/Input.hpp"

namespace Izo {

Widget::Widget() : m_bounds{0, 0, 0, 0}, m_measured_size{0, 0, 0, 0}, m_focus_anim(0.0f), m_prev_touch_down(false), m_touch_started_inside(false), m_focusable(true) {}

void Widget::draw(Painter& painter) {
    if (!m_visible) return;
    draw_content(painter);
}

void Widget::draw_focus(Painter& painter) {
    if (!m_visible) return;
    if (m_focusable && m_show_focus_indicator) {
        draw_focus_outline(painter);
    }
}

void Widget::update() {
    m_focus_anim.update(Application::the().delta());
}

void Widget::measure(int parent_w, int parent_h) {
    int w = 0, h = 0;
    if (m_width == (int)WidgetSizePolicy::MatchParent) w = parent_w;
    else if (m_width > 0) w = m_width;

    if (m_height == (int)WidgetSizePolicy::MatchParent) h = parent_h;
    else if (m_height > 0) h = m_height;

    m_measured_size = {0, 0, w, h};
}

void Widget::handle_focus_logic(bool inside, bool down) {
    bool old_focused = m_focused;

    if (down && !m_prev_touch_down) {
        m_touch_started_inside = inside;
        m_gesture_cancelled = false;

        if (!inside) {
            set_focused(false);
        }
    }

    if (!down && m_prev_touch_down && !m_gesture_cancelled) {
        if (inside && m_touch_started_inside && m_focusable) {
            set_focused(true);
        }
    }

    if (!down) {
        m_touch_started_inside = false;
        m_gesture_cancelled = false;
    }

    m_prev_touch_down = down;
}

bool Widget::on_touch(IntPoint point, bool down, bool captured) {
    if (!m_visible) return false;

    IntRect b = screen_bounds();
    bool inside = b.contains(point);
    handle_focus_logic(inside, down);

    if (!inside && !captured) return false;

    IntPoint local_point = { point.x - b.x, point.y - b.y };

    return on_touch_event(local_point, down);
}

void Widget::draw_focus_outline(Painter& painter) {
    float t = m_focus_anim.value();
    if (t > 0) {
        int max_thickness = ThemeDB::the().int_value("Widget.FocusThickness", 12);
        float expansion = max_thickness * (1.0f - t); 
        uint8_t alpha = (uint8_t)(255 * t);
        Color theme_focus = ThemeDB::the().color("Widget.Focus");
        Color color(theme_focus.r, theme_focus.g, theme_focus.b, alpha);

        IntRect b = screen_bounds();

        int draw_thickness = (int)(max_thickness * (1.0f - t)) + 1;
        if (draw_thickness < 1) draw_thickness = 1;
        int exp_int = (int)expansion;
        
        painter.draw_rounded_rect({ 
            b.x - exp_int, 
            b.y - exp_int, 
            b.w + exp_int * 2, 
            b.h + exp_int * 2 
        }, ThemeDB::the().int_value("Widget.Roundness", 6), color, draw_thickness);
    }
}

void Widget::set_focused(bool focused) {
    if (m_focused != focused) {
        m_focused = focused;
        int anim_duration = ThemeDB::the().int_value("Widget.FocusAnimDuration", 300);
        m_focus_anim.set_target(m_focused ? 1.0f : 0.0f, anim_duration, Easing::EaseOutCubic);
    }
}

bool Widget::hovering() const {
    return screen_bounds().contains(Input::the().touch_point());
}

void Widget::show() { if (!m_visible) { m_visible = true; } }
void Widget::hide() { if (m_visible) { m_visible = false; } }

IntRect Widget::screen_bounds() const {
    IntRect r = m_bounds;
    const Widget* p = m_parent;
    while (p) {
        IntPoint offset = p->content_scroll_offset();
        r.x += offset.x; 
        r.y += offset.y;
        p = p->parent();
    }
    return r;
}

} 