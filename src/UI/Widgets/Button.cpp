#include "Button.hpp"
#include "Core/ThemeDB.hpp"
#include "Input/Input.hpp"
#include "Core/Application.hpp"
#include "Graphics/Painter.hpp"
#include "Graphics/Font.hpp"

namespace Izo {

Button::Button(const std::string& text) 
    : m_label(std::make_unique<Label>(text)),
      m_bg_anim(Color(100))
{
    set_widget_type("Button");
    m_label->set_alignment(TextAlign::Center);
    on_theme_update();
}

void Button::on_theme_update() {
    Widget::on_theme_update();
    m_roundness = ThemeDB::the().get<int>("WidgetParams", "Widget.Roundness", 6);
    m_color_text = ThemeDB::the().get<Color>("Colors", "Button.Text", Color(255));
    m_color_bg = ThemeDB::the().get<Color>("Colors", "Button.Background", Color(100));
    m_color_hover = ThemeDB::the().get<Color>("Colors", "Button.Hover", Color(150));
    m_color_pressed = ThemeDB::the().get<Color>("Colors", "Button.Pressed", Color(100));

    if (m_label) {
        m_label->on_theme_update();
        m_label->set_color(m_color_text);
    }

    Color target = m_color_bg;
    if (m_pressed) {
        target = m_color_pressed;
    } else if (m_is_hovered) {
        target = m_color_hover;
    }
    m_bg_anim.snap_to(target);
    invalidate_layout();
}

void Button::draw_content(Painter& painter) {
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
    if (b.w < 0) b.w = 0;
    if (b.h < 0) b.h = 0;

    painter.fill_rounded_rect(b, m_roundness, c); 
    painter.draw_rounded_rect(b, m_roundness, m_color_text);

    if (m_label) {
        m_label->draw(painter);
    }
}

void Button::update() {
    float dt = Application::the().delta();
    bool old_hovered = m_is_hovered;
    Color old_bg = m_bg_anim.value();
    bool was_anim_running = m_bg_anim.running();

    m_is_hovered = global_bounds().contains(Input::the().touch_point());

    if (!m_pressed && old_hovered != m_is_hovered) {
        if (m_is_hovered) {
            m_bg_anim.set_target(m_color_hover, 200, Easing::EaseOutQuad);
        } else {
            m_bg_anim.set_target(m_color_bg, 200, Easing::EaseOutQuad);
        }
    }

    m_bg_anim.update(dt);

    if (m_label) {
        m_label->set_color(m_color_text);
        
        IntRect b = global_bounds();
        int app_width = Application::the().width();
        int app_height = Application::the().height();
        if (b.x + b.w > app_width) {
            b.w = app_width - b.x;
        }
        if (b.y + b.h > app_height) {
            b.h = app_height - b.y;
        }
        if (b.w < 0) b.w = 0;
        if (b.h < 0) b.h = 0;

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

    if (old_hovered != m_is_hovered ||
        was_anim_running ||
        m_bg_anim.running() ||
        old_bg.as_argb() != m_bg_anim.value().as_argb()) {
        invalidate_visual();
    }
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
        Color target = m_color_bg;
        if (m_pressed) 
            target = m_color_pressed;
        else if (m_is_hovered) 
            target = m_color_hover;

        m_bg_anim.set_target(target, 200, Easing::EaseOutQuad);
        invalidate_visual();
    }

    return true; 
}

bool Button::on_key(KeyCode key) {
    if (!m_focused) return false;

    if (key == KeyCode::Enter) {
        if (m_on_click) m_on_click();
        
        m_bg_anim.snap_to(m_color_pressed);
        m_bg_anim.set_target(m_color_hover, 300, Easing::EaseOutQuad);
        invalidate_visual();
        
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

bool Button::has_running_animations() const {
    return Widget::has_running_animations() || m_bg_anim.running() ||
           (m_label && m_label->has_running_animations());
}

}
