// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "Widget.hpp"
#include "Color.hpp"
#include "Image.hpp"
#include <functional>

namespace Izo {

class Slider : public Widget {
public:
    Slider(Image* handleImage, Image* handleFocusImage, float value = 0.0f);

    void set_value(float v);
    float value() const;
    void set_on_change(std::function<void(float)> callback) { m_on_change = callback; }

    void draw_content(Painter& painter) override;
    void measure(int parent_w, int parent_h) override;
    bool on_touch_event(int local_x, int local_y, bool down) override;

private:
    float m_val;
    Image* m_handle;
    Image* m_handle_focus;
    bool m_pressed = false;
    std::function<void(float)> m_on_change;
};

} // namespace Izo
