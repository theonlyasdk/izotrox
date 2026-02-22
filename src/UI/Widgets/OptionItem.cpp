#include "UI/Widgets/OptionItem.hpp"
#include "Graphics/Painter.hpp"
#include "Graphics/Font.hpp"
#include "Core/ThemeDB.hpp"
#include "Core/Application.hpp"

namespace Izo {

OptionItem::OptionItem(const std::string& text, int index, std::function<void(int)> callback)
    : m_text(text), m_index(index), m_callback(callback),
      m_bg_anim(Color(0)) {
    m_focusable = false;
    set_padding_ltrb(20, 10, 20, 10);
    set_width(WidgetSizePolicy::MatchParent);
    on_theme_update();
}

void OptionItem::on_theme_update() {
    Widget::on_theme_update();
    m_roundness = ThemeDB::the().get<int>("WidgetParams", "Widget.Roundness", 12);
    m_color_highlight = ThemeDB::the().get<Color>("Colors", "OptionBox.Highlight", Color(255, 255, 255, 40));
    m_color_text = ThemeDB::the().get<Color>("Colors", "OptionBox.Text", Color(255));
    invalidate_visual();
}

void OptionItem::measure(int parent_w, int parent_h) {
    if (!m_font) return;
    int h = m_font->height() + m_padding_top + m_padding_bottom;
    m_measured_size = {0, 0, parent_w, h};
}

void OptionItem::draw_content(Painter& painter) {
    IntRect bounds = global_bounds();

    if (m_selected || m_pressed || m_hovered) {
        Color highlight = m_color_highlight;
        if (!m_pressed && !m_selected) highlight.a /= 2;
        painter.fill_rounded_rect(bounds, m_roundness, highlight);
    }

    if (m_font) {
        int ty = bounds.y + (bounds.h - m_font->height()) / 2;
        m_font->draw_text(painter, {bounds.x + m_padding_left, ty}, m_text, m_color_text);
    }
}

void OptionItem::update() {
    bool old_hovered = m_hovered;
    bool was_running = m_bg_anim.running();
    Color old_bg = m_bg_anim.value();

    m_hovered = hovering();
    m_bg_anim.update(Application::the().delta());
    Widget::update();

    if (old_hovered != m_hovered ||
        was_running ||
        m_bg_anim.running() ||
        old_bg.as_argb() != m_bg_anim.value().as_argb()) {
        invalidate_visual();
    }
}

void OptionItem::set_selected(bool selected) {
    if (m_selected == selected) return;
    m_selected = selected;
    invalidate_visual();
}

bool OptionItem::on_touch_event(IntPoint point, bool down) {
    if (down) {
        m_pressed = true;
        invalidate_visual();
    } else {
        if (m_pressed && content_box().contains(point)) {
            m_callback(m_index);
        }
        m_pressed = false;
        invalidate_visual();
    }
    return true;
}

}
