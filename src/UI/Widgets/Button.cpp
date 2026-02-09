#include "Button.hpp"
#include "Core/ThemeDB.hpp"
#include "Input/Input.hpp"
#include "Core/Application.hpp"
#include "Graphics/Painter.hpp"
#include "Graphics/Font.hpp"

namespace Izo {

Button::Button(const std::string& text) 
    : m_label(std::make_unique<Label>(text)),
      m_bg_anim(ThemeDB::the().get<Color>("Colors", "Button.Background", Color(100))) 
{
    m_label->set_alignment(TextAlign::Center);
}

void Button::on_theme_reload() {
    Widget::on_theme_reload();
    if (m_label) 
        m_label->on_theme_reload();
}

void Button::draw_content(Painter& painter) {
    int roundness           = ThemeDB::the().get<int>("Looks", "Widget.Roundness", 6);
    Color color_btn_text    = ThemeDB::the().get<Color>("Colors", "Button.Text", Color(255));
    IntRect b = global_bounds();
    Color c = m_bg_anim.value();

    int app_width = Application::the().width();
    int app_height = Application::the().height();

    if (b.x + b.w > app_width) {
        b.w = app_width - b.x;
    }
    if (b.y + b.h > app_height) {
        b.h = app_height - b.y;
    }

    painter.fill_rounded_rect(b, roundness, c); 
    painter.draw_rounded_rect(b, roundness, color_btn_text);

    if (m_label) {
        m_label->draw(painter);
    }
}

void Button::update() {
    float dt = Application::the().delta();

    Color color_btn_hover   = ThemeDB::the().get<Color>("Colors", "Button.Hover", Color(150));
    Color color_btn_bg      = ThemeDB::the().get<Color>("Colors", "Button.Background", Color(100));
    Color color_btn_text    = ThemeDB::the().get<Color>("Colors", "Button.Text", Color(255));

    m_is_hovered = global_bounds().contains(Input::the().touch_point());

    if (m_is_hovered) {
        m_bg_anim.set_target(color_btn_hover, 200, Easing::EaseOutQuad);
    } else {
        m_bg_anim.set_target(color_btn_bg, 200, Easing::EaseOutQuad);
    }

    m_bg_anim.update(dt);

    if (m_label) {
        m_label->set_color(color_btn_text);
        
        IntRect b = global_bounds();
        int label_h = m_label->measured_height();
        
        int padding_x = 10;
        int target_w = b.w - padding_x;
        if (target_w < 0) target_w = 0;
        
        int ly = b.y + (b.h - label_h) / 2;
        if (m_pressed) ly += 1;
        
        m_label->set_bounds({b.x + padding_x / 2, ly, target_w, label_h});
        m_label->update();
    }
    
    Widget::update();
}

bool Button::on_touch_event(IntPoint point, bool down) {
    bool inside = content_box().contains(point);
    bool old_pressed = m_pressed;
    bool old_hovered = m_is_hovered;

    Color color_btn_hover   = ThemeDB::the().get<Color>("Colors", "Button.Hover", Color(150));
    Color color_btn_bg      = ThemeDB::the().get<Color>("Colors", "Button.Background", Color(100));
    Color color_btn_pressed = ThemeDB::the().get<Color>("Colors", "Button.Pressed", Color(100));

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
        Color target = color_btn_bg;
        if (m_pressed) 
            target = color_btn_pressed;
        else if (m_is_hovered) 
            target = color_btn_hover;

        m_bg_anim.set_target(target, 200, Easing::EaseOutQuad);
    }

    return true; 
}

bool Button::on_key(KeyCode key) {
    if (!m_focused) return false;

    Color color_btn_hover   = ThemeDB::the().get<Color>("Colors", "Button.Hover", Color(150));
    Color color_btn_pressed = ThemeDB::the().get<Color>("Colors", "Button.Pressed", Color(100));

    if (key == KeyCode::Enter) {
        if (m_on_click) m_on_click();
        
        m_bg_anim.snap_to(color_btn_pressed);
        m_bg_anim.set_target(color_btn_hover, 300, Easing::EaseOutQuad);
        
        return true;
    }
    return false;
}

void Button::measure(int parent_w, int parent_h) {
    int mw = 50, mh = 20;
    if (m_label) {
        m_label->measure(parent_w, parent_h);
        mw = m_label->measured_width() + 20;
        mh = m_label->measured_height() + 10;
    }
    m_measured_size = {0, 0, mw, mh};
}

}