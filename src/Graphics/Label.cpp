// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Label.hpp"
#include "Core/ThemeDB.hpp"

namespace Izo {

Label::Label(const std::string& text, Font* font, Color color) 
    : m_text_str(text), m_font(font), m_color(color) {}

void Label::draw_content(Painter& painter) {
    if (m_font) {
        int maxW = m_wrap ? m_bounds.w : -1;
        m_font->draw_text_multiline(painter, m_bounds.x, m_bounds.y, m_text_str, m_color, maxW);
    }
}

void Label::measure(int parent_w, int parent_h) {
    int mw = 0, mh = 0;
    if (m_font) {
        int maxW = -1;
        if (m_wrap) {
            // If wrapping, we need a width limit.
            // If our policy is MatchParent, use parent_w.
            // If Fixed, use m_width (if we had it stored, but we only have policy).
            // Actually Layout passes `parent_w` as the available space.
            // If we are in a LinearLayout, parent_w might be the container width.
            maxW = parent_w; 
            
            // Adjust for padding if we had it.
        }
        
        m_font->measure_multiline(m_text_str, mw, mh, maxW);
    }
    m_measured_size = {0, 0, mw, mh};
}

} // namespace Izo