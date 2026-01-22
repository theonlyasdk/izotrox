// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Button.hpp"
#include "Core/ThemeDB.hpp"
#include <cmath>

namespace Izo {

Button::Button(const std::string& text, Font* font) 
    : m_text_str(text), m_font(font), 
      m_bg_anim(ThemeDB::the().color("Button.Background")) {}

void Button::draw_content(Painter& painter) {
    Color c = m_bg_anim.value();
    painter.fill_rect(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h, c);
    painter.draw_rect(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h, ThemeDB::the().color("Button.Text"));

    if (m_font) {
        int tw = m_font->width(m_text_str);
        int th = m_font->height();
        int tx = m_bounds.x + (m_bounds.w - tw) / 2;
        int ty = m_bounds.y + (m_bounds.h - th) / 2;
        
        if (m_pressed) ty += 1;
        
        m_font->draw_text(painter, tx, ty, m_text_str, ThemeDB::the().color("Button.Text"));
    }
}

void Button::update() {
    m_bg_anim.update(16.0f);
    Widget::update(); 
}

bool Button::on_touch_event(int local_x, int local_y, bool down) {
    // Bounds check: 0 <= local_x < w
    bool inside = (local_x >= 0 && local_x < m_bounds.w && local_y >= 0 && local_y < m_bounds.h);
    
    bool old_pressed = m_pressed;
    bool old_hovered = m_is_hovered;
    
    m_is_hovered = inside; 

    if (inside) {
        m_pressed = down;
    } else {
        m_pressed = false;
    }
    
    if (old_pressed && !m_pressed && inside && !down) {
        if (m_on_click) m_on_click();
    }
    
    if (old_pressed != m_pressed || old_hovered != m_is_hovered) {
        Color target = ThemeDB::the().color("Button.Background");
        if (m_pressed) target = ThemeDB::the().color("Button.Pressed");
        else if (m_is_hovered) target = ThemeDB::the().color("Button.Hover");
        
        m_bg_anim.set_target(target, 200, Easing::EaseOutQuad);
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