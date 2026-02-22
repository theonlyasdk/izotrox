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
    Button(const std::string& text);

    void on_theme_update() override;

    void draw_content(Painter& painter) override;
    void update() override;
    bool on_touch_event(IntPoint point, bool down) override;
    bool on_key(KeyCode key) override;
    void measure(int parent_w, int parent_h) override;
    bool has_running_animations() const override;

    void set_on_click(std::function<void()> callback) { m_on_click = callback; }

private:
    std::unique_ptr<Label> m_label;
    bool m_pressed = false;
    bool m_is_hovered = false;

    Animator<Color> m_bg_anim;

    int m_roundness = 6;
    Color m_color_text{255, 255, 255};
    Color m_color_bg{100, 100, 100};
    Color m_color_hover{150, 150, 150};
    Color m_color_pressed{100, 100, 100};
    
    std::function<void()> m_on_click;
};

} 
