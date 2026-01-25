

#pragma once
#include "Widget.hpp"
#include "Font.hpp"
#include "Color.hpp"
#include "Animator.hpp"
#include <string>
#include <functional>

namespace Izo {

class Button : public Widget {
public:
    Button(const std::string& text, Font* font);

    void draw_content(Painter& painter) override;
    void update() override;
    bool on_touch_event(IntPoint point, bool down) override;
    void measure(int parent_w, int parent_h) override;

    void set_on_click(std::function<void()> callback) { m_on_click = callback; }

private:
    std::string m_text_str;
    Font* m_font;
    bool m_pressed = false;
    bool m_is_hovered = false;

    Animator<Color> m_bg_anim;

    std::function<void()> m_on_click;
};

} 