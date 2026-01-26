

#include "ProgressBar.hpp"
#include "Core/ThemeDB.hpp"
#include <algorithm>

namespace Izo {

ProgressBar::ProgressBar(float progress) : m_value(progress) {
    set_focusable(false);
}

void ProgressBar::set_progress(float v) { 
    float new_progress = std::clamp(v, 0.0f, 1.0f);
    if (new_progress != m_value) {
        m_value = new_progress;
    }
}

float ProgressBar::progress() const { return m_value; }

void ProgressBar::draw_content(Painter& painter) {
    IntRect b = global_bounds();
    int roundness = ThemeDB::the().int_value("Widget.Roundness", 6);
    
    painter.fill_rounded_rect(b, roundness, ThemeDB::the().color("ProgressBar.Background"));

    if (m_value > 0.0f) {
        int fill_w = static_cast<int>(b.w * m_value);
        if (fill_w > 0) {
            // We use clipping to ensure the fill respects the rounded corners of the background
            painter.push_clip({b.x, b.y, fill_w, b.h});
            painter.fill_rounded_rect(b, roundness, ThemeDB::the().color("ProgressBar.Fill"));
            painter.pop_clip();
        }
    }
}

void ProgressBar::measure(int parent_w, int parent_h) {
    m_measured_size = {0, 0, 100, 20};
}

bool ProgressBar::on_touch_event(IntPoint point, bool down) {
     return false; 
}

} 