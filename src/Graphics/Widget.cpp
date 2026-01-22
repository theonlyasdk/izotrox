// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Widget.hpp"
#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include <cstdlib>

namespace Izo {

std::vector<IntRect> Widget::s_dirty_rects;

Widget::Widget() : m_bounds{0, 0, 0, 0}, m_focus_anim(0.0f), m_is_focusable(true) {}

void Widget::draw(Painter& painter) {
    if (!m_visible) return;

    draw_content(painter);

    if (m_is_focusable && m_show_focus_indicator) {
        draw_focus_outline(painter);
    }
}

void Widget::update() {
    if (m_focus_anim.update(Application::the().delta())) 
        invalidate();
}

void Widget::measure(int parent_w, int parent_h) {
    m_measured_size = {0, 0, 0, 0};
}

void Widget::handle_focus_logic(bool inside, bool down) {
    bool old_focused = m_is_focused;

    if (down) {
        if (inside && !m_prev_touch_down) { 
            if (m_is_focusable) m_is_focused = true;
        } else if (!inside && !m_prev_touch_down) {
            // Clicked outside - lose focus
            m_is_focused = false;
        }
    }
    
    if (!down) m_prev_touch_down = false;
    else m_prev_touch_down = true;
    
    if (old_focused != m_is_focused) {
        int anim_duration = ThemeDB::the().int_value("Widget.FocusAnimDuration", 6);
        m_focus_anim.set_target(m_is_focused ? 1.0f : 0.0f, anim_duration, Easing::EaseOutCubic);
        invalidate();
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
    if (m_is_focused != focused) {
        m_is_focused = focused;
        invalidate();
    }
}

void Widget::show() { if (!m_visible) { m_visible = true; invalidate(); } }
void Widget::hide() { if (m_visible) { m_visible = false; invalidate(); } }

void Widget::invalidate() { 
    IntRect rect = m_bounds;
    if (m_is_focusable && m_show_focus_indicator) {
        int margin = ThemeDB::the().int_value("Widget.FocusThickness", 6);
        rect.x -= margin;
        rect.y -= margin;
        rect.w += margin * 2;
        rect.h += margin * 2;
    }
    add_dirty_rect(rect);
}

bool Widget::dirty() { return !s_dirty_rects.empty(); }
void Widget::clear_dirty() { s_dirty_rects.clear(); }

const std::vector<IntRect>& Widget::get_dirty_rects() {
    return s_dirty_rects;
}

void Widget::add_dirty_rect(const IntRect& rect) {
    s_dirty_rects.push_back(rect);
}

} // namespace Izo