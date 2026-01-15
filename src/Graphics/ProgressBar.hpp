// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "Widget.hpp"
#include "Color.hpp"

namespace Izo {

class ProgressBar : public Widget {
public:
    ProgressBar(float progress = 0.0f);

    void set_progress(float v);
    float progress() const;

    void draw_content(Painter& painter) override;
    void measure(int parent_w, int parent_h) override;
    bool on_touch_event(int, int, bool) override;

private:
    float m_val;
};

} // namespace Izo
