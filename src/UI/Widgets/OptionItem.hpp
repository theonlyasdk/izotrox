#pragma once

#include "UI/Widgets/Widget.hpp"
#include "Graphics/Color.hpp"
#include <string>
#include <functional>

namespace Izo {

class OptionItem : public Widget {
public:
    OptionItem(const std::string& text, int index, std::function<void(int)> callback);

    void draw_content(Painter& painter) override;
    bool on_touch_event(IntPoint point, bool down) override;
    void measure(int parent_w, int parent_h) override;

    void set_selected(bool selected) { m_selected = selected; }
    bool is_selected() const { return m_selected; }

private:
    std::string m_text;
    int m_index;
    std::function<void(int)> m_callback;
    bool m_selected = false;
    bool m_pressed = false;
    Animator<Color> m_bg_anim;
};

}
