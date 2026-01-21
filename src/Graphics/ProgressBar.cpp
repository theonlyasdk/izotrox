// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "ProgressBar.hpp"
#include "Core/ThemeDB.hpp"
#include <algorithm>

namespace Izo {

ProgressBar::ProgressBar(float progress) : m_val(progress) {}

void ProgressBar::set_progress(float v) { 
    float new_p = std::clamp(v, 0.0f, 1.0f);
    if (new_p != m_val) {
        m_val = new_p;
        Widget::invalidate();
    }
}

float ProgressBar::progress() const { return m_val; }

void ProgressBar::draw_content(Painter& painter) {
    painter.fill_rounded_rect(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h, 4, ThemeDB::the().color("ProgressBar.Background"));
    painter.draw_rounded_rect(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h, 4, Color::White); 

    if (m_val > 0.0f) {
        int fill_w = static_cast<int>(m_bounds.w * m_val);
        painter.fill_rounded_rect(m_bounds.x, m_bounds.y, fill_w, m_bounds.h, 4, ThemeDB::the().color("ProgressBar.Fill"));
    }
}

void ProgressBar::measure(int parent_w, int parent_h) {
    m_measured_size = {0, 0, 100, 20};
}

bool ProgressBar::on_touch_event(int, int, bool) {
     return false; 
}

} // namespace Izo