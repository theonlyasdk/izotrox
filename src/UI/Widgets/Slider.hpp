#pragma once

#include "UI/Widgets/Widget.hpp"
#include "Graphics/Color.hpp"
#include "Graphics/Image.hpp"

#include <functional>

namespace Izo {

class Slider : public Widget {
public:
    Slider(float value = 0.0f);

    void set_value(float v);
    float value() const;
    void set_on_change(std::function<void(float)> callback) { m_on_change = callback; }

    void draw_content(Painter& painter) override;
    void measure(int parent_w, int parent_h) override;
    bool on_touch_event(IntPoint point, bool down) override;

private:
    float m_value;
    Image* m_handle;
    Image* m_handle_focus;
    bool m_pressed = false;
    std::function<void(float)> m_on_change;
};

} 
