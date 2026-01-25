// Mozilla Public License version 2.0. (c) theonlyasdk 2026

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
    IntRect b = bounds();
    painter.fill_rect(b, ThemeDB::the().color("ProgressBar.Background"));
    painter.draw_rect(b, Color::White); 

    if (m_value > 0.0f) {
        int fill_w = static_cast<int>(b.w * m_value);
        painter.fill_rect({b.x, b.y, fill_w, b.h}, ThemeDB::the().color("ProgressBar.Fill"));
    }
}

void ProgressBar::measure(int parent_w, int parent_h) {
    m_measured_size = {0, 0, 100, 20};
}

bool ProgressBar::on_touch_event(IntPoint point, bool down) {
     return false; 
}

} // namespace Izo