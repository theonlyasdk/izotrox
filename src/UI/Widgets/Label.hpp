#pragma once

#include "UI/Widgets/Widget.hpp"
#include "Graphics/ColorVariant.hpp"
#include "Graphics/Color.hpp"
#include "UI/Enums.hpp"

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
    
    void set_alignment(TextAlign align) { m_alignment = align; }
    TextAlign alignment() const { return m_alignment; }
    
    void set_color_variant(ColorVariant variant) { m_color_variant = variant; }
    ColorVariant color_variant() const { return m_color_variant; }

    void set_color(const Color& color);
    Color color() const;

    void update() override;

private:
    std::string m_text;
    Font* m_font;
    bool m_should_wrap = false;
    TextAlign m_alignment = TextAlign::Left;
    ColorVariant m_color_variant = ColorVariant::Default;
    
    // Custom color
    bool m_has_custom_color = false;
    Color m_custom_color{0};

    // Marquee scrolling
    Animator<float> m_scroll_anim{0.0f};
    bool m_should_scroll = false;
};

}