#pragma once

#include "UI/Widgets/Widget.hpp"
#include <functional>

namespace Izo {

class Image;

class Slider : public Widget {
public:
    Slider(float value = 0.0f);

    void set_value(float v);
    float value() const { return m_value; }

    void set_on_change(std::function<void(float)> callback) { m_on_change = callback; }

    void draw_content(Painter& painter) override;
    void measure(int parent_w, int parent_h) override;
    bool on_touch_event(IntPoint point, bool down) override;
    void on_theme_update() override;

private:
    float m_value;
    int m_roundness = 6;
    bool m_pressed = false;
    Image* m_img_handle;
    Image* m_img_handle_focus;
    std::function<void(float)> m_on_change;
    Color m_color_track{90, 90, 90};
    Color m_color_active{90, 90, 90};
};

} 
