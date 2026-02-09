#pragma once

#include "Graphics/Color.hpp"
#include "UI/Widgets/Widget.hpp"
#include "Motion/Animator.hpp"

#include <memory>
#include <string>
#include <functional>
#include "UI/Widgets/Label.hpp"

namespace Izo {

class Font;

class Button : public Widget {
public:
    Button(const std::string& text, Font* font);

    void draw_content(Painter& painter) override;
    void update() override;
    bool on_touch_event(IntPoint point, bool down) override;
    bool on_key(KeyCode key) override;
    void measure(int parent_w, int parent_h) override;

    void set_on_click(std::function<void()> callback) { m_on_click = callback; }

private:
    std::unique_ptr<Label> m_label;
    bool m_pressed = false;
    bool m_is_hovered = false;

    Animator<Color> m_bg_anim;
    
    std::function<void()> m_on_click;
};

} 