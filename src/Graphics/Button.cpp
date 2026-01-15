// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Button.hpp"
#include "../Core/Theme.hpp"
#include <cmath>

namespace Izo {

Button::Button(const std::string& text, Font* font) 
    : m_text_str(text), m_font(font), 
      m_bg_anim(Theme::instance().color("Button.Background")) {}

void Button::draw_content(Painter& painter) {
    Color c = m_bg_anim.value();
    painter.fill_rounded_rect(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h, 6, c);
    painter.draw_rounded_rect(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h, 6, Theme::instance().color("Button.Text"));
    
    if (m_font) {
        int tw = m_font->width(m_text_str);
        int th = m_font->height();
        int tx = m_bounds.x + (m_bounds.w - tw) / 2;
        int ty = m_bounds.y + (m_bounds.h - th) / 2;
        
        if (m_is_pressed) ty += 1;
        
        m_font->draw_text(painter, tx, ty, m_text_str, Theme::instance().color("Button.Text"));
    }
}

void Button::update() {
    if (m_bg_anim.update(16.0f)) Widget::invalidate();
    Widget::update(); 
}

bool Button::on_touch_event(int local_x, int local_y, bool down) {
    // local_x/y are relative to m_bounds.
    // inside check is implicit by on_touch calling us?
    // Widget::on_touch checks bounds before calling handle_focus_logic.
    // But it passes event to us regardless of inside if we are captured?
    // Widget::on_touch only checks inside for focus logic.
    // It passes `local_x` which might be outside.
    // So we need to check inside.
    
    // Bounds check: 0 <= local_x < w
    bool inside = (local_x >= 0 && local_x < m_bounds.w && local_y >= 0 && local_y < m_bounds.h);
    
    bool old_pressed = m_is_pressed;
    bool old_hovered = m_is_hovered;
    
    m_is_hovered = inside; 

    if (inside) {
        m_is_pressed = down;
    } else {
        m_is_pressed = false;
    }
    
    if (old_pressed && !m_is_pressed && inside && !down) {
        if (m_on_click) m_on_click();
    }
    
    if (old_pressed != m_is_pressed || old_hovered != m_is_hovered) {
        Color target = Theme::instance().color("Button.Background");
        if (m_is_pressed) target = Theme::instance().color("Button.Pressed");
        else if (m_is_hovered) target = Theme::instance().color("Button.Hover");
        
        m_bg_anim.set_target(target, 200, Easing::EaseOutQuad);
        Widget::invalidate();
    }
    
    // Focus handled by base Widget
    return true; // Consume event if interactive
}

void Button::measure(int parent_w, int parent_h) {
     int mw = 50, mh = 20;
     if (m_font) {
        mw = m_font->width(m_text_str) + 20;
        mh = m_font->height() + 10;
    }
    m_measured_size = {0, 0, mw, mh};
}

} // namespace Izo
