// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Widget.hpp"
#include "Core/ThemeDB.hpp"

namespace Izo {

bool Widget::s_global_dirty = true;

Widget::Widget() : m_bounds{0, 0, 0, 0}, m_focus_anim(0.0f), m_is_focusable(true) {}

void Widget::draw(Painter& painter) {
    if (!m_visible) return;
    
    draw_content(painter);

    if (m_is_focusable && m_show_focus_indicator) {
        draw_focus_outline(painter);
    }
}

void Widget::update() {
    if (m_focus_anim.update(16.0f)) invalidate();
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
        m_focus_anim.set_target(m_is_focused ? 1.0f : 0.0f, 300, Easing::EaseOutQuad);
        invalidate();
    }
}

bool Widget::on_touch(int tx, int ty, bool down, bool captured) {
    if (!m_visible) return false;
    
    bool inside = m_bounds.contains(tx, ty);
    handle_focus_logic(inside, down);
    
    if (!inside && !captured) return false;
    
    // Pass event to subclass
    int local_x = tx - m_bounds.x;
    int local_y = ty - m_bounds.y;
    
    return on_touch_event(local_x, local_y, down);
}

void Widget::draw_focus_outline(Painter& painter) {
    float t = m_focus_anim.value();
    if (t > 0.01f) {
        int max_thickness = ThemeDB::the().value("Widget.FocusThickness", 6);
        int radius = ThemeDB::the().value("Widget.FocusRadius", 4); 
        
        float expansion = (max_thickness * (1.0f - t)); 
        float thickness = expansion + 1;
        
        uint8_t alpha = (uint8_t)(255 * t);
        Color theme_focus = ThemeDB::the().color("Widget.Focus");
        Color color(theme_focus.r, theme_focus.g, theme_focus.b, alpha);
        
        // Using the new draw_rounded_rect with thickness to avoid overlaps
        painter.draw_rect(m_bounds.x - expansion, m_bounds.y - expansion, 
                                  m_bounds.w + expansion*2, m_bounds.h + expansion*2, color);
    }
}

void Widget::set_focused(bool focused) {
    if (m_is_focused != focused) {
        m_is_focused = focused;
        m_focus_anim.set_target(m_is_focused ? 1.0f : 0.0f, 300, Easing::EaseOutQuad);
        invalidate();
    }
}

void Widget::show() { if (!m_visible) { m_visible = true; invalidate(); } }
void Widget::hide() { if (m_visible) { m_visible = false; invalidate(); } }

void Widget::invalidate() { s_global_dirty = true; }
bool Widget::dirty() { return s_global_dirty; }
void Widget::clear_dirty() { s_global_dirty = false; }

} // namespace Izo