// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Button.hpp"
#include "Core/ThemeDB.hpp"
#include "Input/Input.hpp"

namespace Izo {

Button::Button(const std::string& text, Font* font) 
    : m_text_str(text), m_font(font), 
      m_bg_anim(ThemeDB::the().color("Button.Background")) {}

void Button::draw_content(Painter& painter) {
    IntRect b = bounds();
    Color c = m_bg_anim.value();
    painter.fill_rect(b, c);
    painter.draw_rect(b, ThemeDB::the().color("Button.Text"));

    if (m_font) {
        int tw = m_font->width(m_text_str);
        int th = m_font->height();
        int tx = b.x + (b.w - tw) / 2;
        int ty = b.y + (b.h - th) / 2;
        
        if (m_pressed) ty += 1;
        
        m_font->draw_text(painter, {tx, ty}, m_text_str, ThemeDB::the().color("Button.Text"));
    }
}

void Button::update() {
    m_is_hovered = bounds().contains(Input::the().touch_point());

    if (m_is_hovered) {
        m_bg_anim.set_target(ThemeDB::the().color("Button.Hover"), 200, Easing::EaseOutQuad);
    } else {
        m_bg_anim.set_target(ThemeDB::the().color("Button.Background"), 200, Easing::EaseOutQuad);
    }

    m_bg_anim.update(16.0f);
    Widget::update();
}

bool Button::on_touch_event(IntPoint point, bool down) {
    bool inside = content_box().contains(point);

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