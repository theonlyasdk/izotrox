// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "Widget.hpp"
#include "Font.hpp"
#include "Color.hpp"
#include <string>

namespace Izo {

class Label : public Widget {
public:
    Label(const std::string& text, Font* font, Color color);

    void draw_content(Painter& painter) override;
    void measure(int parent_w, int parent_h) override;

private:
    std::string m_text_str;
    Font* m_font;
    Color m_color;
};

} // namespace Izo