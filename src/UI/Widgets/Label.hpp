#pragma once

#include "UI/Widgets/Widget.hpp"
#include "Graphics/ColorVariant.hpp"

#include <string>

namespace Izo {

class Font;

class Label : public Widget {
public:
    Label(const std::string& text, Font* font);

    void draw_content(Painter& painter) override;
    void measure(int parent_w, int parent_h) override;

    void set_font(Font* font) { m_font = font; }
    void set_wrap(bool wrap) { m_should_wrap = wrap; }
    
    void set_color_variant(ColorVariant variant) { m_color_variant = variant; }
    ColorVariant color_variant() const { return m_color_variant; }

private:
    std::string m_text;
    Font* m_font;
    bool m_should_wrap = false;
    ColorVariant m_color_variant = ColorVariant::Default;
};

}