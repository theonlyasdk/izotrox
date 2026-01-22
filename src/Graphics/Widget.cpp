// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Widget.hpp"
#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include <cstdlib>

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

    // On initial press
    if (down && !m_prev_touch_down) {
        m_touch_started_inside = inside;
        m_gesture_cancelled = false;
        // Lose focus if clicking outside
        if (!inside) {
            m_focused = false;
        }
    }

    // On release - only focus if gesture wasn't cancelled and started inside
    if (!down && m_prev_touch_down && !m_gesture_cancelled) {
        if (inside && m_touch_started_inside && m_focusable) {
            m_focused = true;
        }
    }
    
    // Reset on release
    if (!down) {
        m_touch_started_inside = false;
        m_gesture_cancelled = false;
    }
    
    m_prev_touch_down = down;
    
    if (old_focused != m_focused) {
        int anim_duration = ThemeDB::the().int_value("Widget.FocusAnimDuration", 6);
        m_focus_anim.set_target(m_focused ? 1.0f : 0.0f, anim_duration, Easing::EaseOutCubic);
    }
}

bool Widget::on_touch(int tx, int ty, bool down, bool captured) {
    if (!m_visible) return false;
    
    bool inside = m_bounds.contains(tx, ty);
    handle_focus_logic(inside, down);
    
    if (!inside && !captured) return false;

    int local_x = tx - m_bounds.x;
    int local_y = ty - m_bounds.y;
    
    return on_touch_event(local_x, local_y, down);
}

void Widget::draw_focus_outline(Painter& painter) {
    float t = m_focus_anim.value();
    if (t > 0) {
        int max_thickness = ThemeDB::the().int_value("Widget.FocusThickness", 6);
        float expansion = max_thickness * (1 - t); 
        uint8_t alpha = (uint8_t)(255 * t);
        Color theme_focus = ThemeDB::the().color("Widget.Focus");
        Color color(theme_focus.r, theme_focus.g, theme_focus.b, alpha);

        for (int i = 0; i < max_thickness; i++) {
            float new_exp = expansion * (max_thickness - i)/max_thickness;
            painter.draw_rect(m_bounds.x - new_exp, m_bounds.y - new_exp, m_bounds.w + new_exp*2, m_bounds.h + new_exp*2, color);
        }
    }
}

void Widget::set_focused(bool focused) {
    if (m_focused != focused) {
        m_focused = focused;
    }
}

void Widget::show() { if (!m_visible) { m_visible = true; } }
void Widget::hide() { if (m_visible) { m_visible = false; } }

} // namespace Izo