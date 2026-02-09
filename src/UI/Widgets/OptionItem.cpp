#include "UI/Widgets/OptionItem.hpp"
#include "Graphics/Painter.hpp"
#include "Graphics/Font.hpp"
#include "Core/ThemeDB.hpp"
#include "Input/Input.hpp"
#include "Core/Application.hpp"

namespace Izo {

OptionItem::OptionItem(const std::string& text, int index, std::function<void(int)> callback)
    : m_text(text), m_index(index), m_callback(callback),
      m_bg_anim(Color(0, 0, 0, 0)) {
    m_focusable = true;
    set_padding_ltrb(20, 10, 20, 10);
    set_width(WidgetSizePolicy::MatchParent);
}

void OptionItem::measure(int parent_w, int parent_h) {
    if (!m_font) return;
    int h = m_font->height() + m_padding_top + m_padding_bottom;
    m_measured_size = {0, 0, parent_w, h};
}

void OptionItem::draw_content(Painter& painter) {
    auto roundness = ThemeDB::the().get<int>("Looks", "Widget.Roundness", 12);
    auto color_highlight = ThemeDB::the().get<Color>("Colors", "OptionBox.Highlight", Color(255, 255, 255, 40));
    auto color_text = ThemeDB::the().get<Color>("Colors", "OptionBox.Text", Color(255));
    
    IntRect bounds = global_bounds();
    
    m_bg_anim.update(Application::the().delta());
    
    if (m_selected || m_pressed || hovering()) {
        Color highlight = color_highlight;
        if (!m_pressed && !m_selected) highlight.a /= 2;
        painter.fill_rounded_rect(bounds, roundness, highlight);
    }

    if (m_font) {
        int ty = bounds.y + (bounds.h - m_font->height()) / 2;
        m_font->draw_text(painter, {bounds.x + m_padding_left, ty}, m_text, color_text);
    }
}

bool OptionItem::on_touch_event(IntPoint point, bool down) {
    if (down) {
        m_pressed = true;
    } else {
        if (m_pressed && content_box().contains(point)) {
            m_callback(m_index);
        }
        m_pressed = false;
    }
    return true;
}

}
