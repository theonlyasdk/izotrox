#include "Button.hpp"
#include "Core/ThemeDB.hpp"
#include "Input/Input.hpp"
#include "Core/Application.hpp"

namespace Izo {

Button::Button(const std::string& text, Font* font) 
    : m_text_str(text), m_font(font), 
      m_bg_anim(ThemeDB::the().color("Button.Background")) {}

void Button::draw_content(Painter& painter) {
    IntRect b = screen_bounds();
    Color c = m_bg_anim.value();
    int roundness = ThemeDB::the().int_value("Widget.Roundness", 6);
    painter.fill_rounded_rect(b, roundness, c); 
    painter.draw_rounded_rect(b, roundness, ThemeDB::the().color("Button.Text"));

    if (m_font) {
        int tw = m_font->width(m_text_str);
        int th = m_font->height();
        
        painter.push_clip(b);
        
        int tx;
        if (m_should_scroll) {
            tx = b.x + (int)m_scroll_anim.value();
        } else {
            tx = b.x + (b.w - tw) / 2;
        }
        
        int ty = b.y + (b.h - th) / 2;
        if (m_pressed) ty += 1;

        m_font->draw_text(painter, {tx, ty}, m_text_str, ThemeDB::the().color("Button.Text"));
        
        painter.pop_clip();
    }
}

void Button::update() {
    float dt = Application::the().delta();
    m_is_hovered = screen_bounds().contains(Input::the().touch_point());

    if (m_is_hovered) {
        m_bg_anim.set_target(ThemeDB::the().color("Button.Hover"), 200, Easing::EaseOutQuad);
    } else {
        m_bg_anim.set_target(ThemeDB::the().color("Button.Background"), 200, Easing::EaseOutQuad);
    }

    if (m_font) {
        int tw = m_font->width(m_text_str);
        if (tw > m_bounds.w - 10) {
            if (!m_should_scroll) {
                m_should_scroll = true;
                float start_val = 5.0f; 
                float target_val = -(float)tw;
                float distance = start_val - target_val;
                float speed = 50.0f; 
                float duration = (distance / speed) * 1000.0f;
                
                m_scroll_anim = Animator<float>(start_val);
                m_scroll_anim.set_loop(true);
                m_scroll_anim.set_target(target_val, duration, Easing::Linear);
            }
        } else {
            m_should_scroll = false;
        }
    }

    m_scroll_anim.update(dt);
    m_bg_anim.update(dt);
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

    return true; 
}

bool Button::on_key(KeyCode key) {
    if (!m_focused) return false;

    if (key == KeyCode::Enter) {
        if (m_on_click) m_on_click();
        
        m_bg_anim.snap_to(ThemeDB::the().color("Button.Pressed"));
        m_bg_anim.set_target(ThemeDB::the().color("Button.Hover"), 300, Easing::EaseOutQuad);
        
        return true;
    }
    return false;
}

void Button::measure(int parent_w, int parent_h) {
     int mw = 50, mh = 20;
     if (m_font) {
        mw = m_font->width(m_text_str) + 20;
        mh = m_font->height() + 10;
    }
    m_measured_size = {0, 0, mw, mh};
}

} 