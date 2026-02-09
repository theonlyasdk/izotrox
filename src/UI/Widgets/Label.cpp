#include "UI/Widgets/Label.hpp"
#include "Core/ThemeDB.hpp"
#include "Graphics/Painter.hpp"
#include "Graphics/Font.hpp"

#include "Core/Application.hpp"

namespace Izo {

Label::Label(const std::string& text, Font* font) : m_text(text), m_font(font) {}

void Label::set_color(const Color& color) {
    m_has_custom_color = true;
    m_custom_color = color;
}

Color Label::color() const {
    if (m_has_custom_color) return m_custom_color;
    return ThemeDB::the().get_variant_color(m_color_variant);
}

void Label::update() {
    float dt = Application::the().delta();
    
    // Only scroll if wrapping is OFF and there are no newlines and text is wider than bounds
    if (m_font && !m_should_wrap && m_text.find('\n') == std::string::npos) {
        int tw = m_font->width(m_text);
        if (tw > m_bounds.w) {
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
    } else {
        m_should_scroll = false;
    }

    m_scroll_anim.update(dt);
    Widget::update();
}

void Label::draw_content(Painter& painter) {
    Color c = color();

    if (m_font) {
        IntRect bounds = global_bounds();

        if (m_should_scroll) {
            painter.push_clip(bounds);
            int tx = bounds.x + (int)m_scroll_anim.value();
            m_font->draw_text(painter, {tx, bounds.y}, m_text, c);
            painter.pop_clip();
        } else {
            int wrap_w = m_should_wrap ? bounds.w : -1;
            m_font->draw_text_multiline(painter, {bounds.x, bounds.y}, m_text, c, wrap_w, bounds.w, m_alignment);
        }
    }
}

void Label::measure(int parent_w, int parent_h) {
    int mw = 0, mh = 0;
    if (m_font) {
        int wrap_w = -1;
        if (m_should_wrap) wrap_w = parent_w;

        m_font->measure_multiline(m_text, mw, mh, wrap_w);
    }

    // Basic Widget policy implementation
    if (m_width == (int)WidgetSizePolicy::MatchParent)
        mw = parent_w;
    else if (m_width > 0)
        mw = m_width;

    if (m_height == (int)WidgetSizePolicy::MatchParent)
        mh = parent_h;
    else if (m_height > 0)
        mh = m_height;

    m_measured_size = {0, 0, mw, mh};
}

}