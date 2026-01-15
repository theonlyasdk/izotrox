// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Label.hpp"
#include "../Core/Theme.hpp"

namespace Izo {

Label::Label(const std::string& text, Font* font, Color color) 
    : m_text_str(text), m_font(font), m_color(color) {}

void Label::draw_content(Painter& painter) {
    if (m_font) {
        m_font->draw_text(painter, m_bounds.x, m_bounds.y, m_text_str, m_color);
    }
}

void Label::measure(int parent_w, int parent_h) {
    int mw = 0, mh = 0;
    if (m_font) {
        mw = m_font->width(m_text_str);
        mh = m_font->height();
    }
    m_measured_size = {0, 0, mw, mh};
}

} // namespace Izo
