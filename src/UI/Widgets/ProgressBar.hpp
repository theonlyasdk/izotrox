#pragma once

#include "UI/Widgets/Widget.hpp"

namespace Izo {

class ProgressBar : public Widget {
public:
    ProgressBar(float progress = 0.0f);

    void set_progress(float v);
    float progress() const;

    void draw_content(Painter& painter) override;
    void measure(int parent_w, int parent_h) override;
    bool on_touch_event(IntPoint point, bool down) override;

private:
    float m_value;
};

} 
