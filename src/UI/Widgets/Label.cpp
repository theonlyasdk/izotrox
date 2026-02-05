

#include "UI/Widgets/Label.hpp"
#include "Core/ThemeDB.hpp"

namespace Izo {

Label::Label(const std::string& text, Font* font) 
    : m_text_str(text), m_font(font) {}

void Label::draw_content(Painter& painter) {
    Color color = ThemeDB::the().get_variant_color(m_color_variant);

    if (m_font) {
        IntRect bounds = global_bounds();
        int maxW = m_wrap ? bounds.w : -1;
        m_font->draw_text_multiline(painter, {bounds.x, bounds.y}, m_text_str, color, maxW);
    }
}

void Label::measure(int parent_w, int parent_h) {
    int mw = 0, mh = 0;
    if (m_font) {
        int max_w = -1;
        if (m_wrap) max_w = parent_w;

        m_font->measure_multiline(m_text_str, mw, mh, max_w);
    }
    m_measured_size = {0, 0, mw, mh};
}

} 