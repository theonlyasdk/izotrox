#include "ProgressBar.hpp"
#include <algorithm>

namespace Izo {

ProgressBar::ProgressBar(float progress) : progress_(progress), color_(Color::Green), bg_color_(Color(50, 50, 50)) {}

void ProgressBar::set_progress(float v) { 
    float new_p = std::clamp(v, 0.0f, 1.0f);
    if (new_p != progress_) {
        progress_ = new_p;
        Widget::invalidate();
    }
}

float ProgressBar::get_progress() const { return progress_; }

void ProgressBar::draw(Painter& painter) {
    // Draw background
    painter.fill_rect(x, y, w, h, bg_color_);
    painter.draw_rect(x, y, w, h, Color::White);

    // Draw progress
    if (progress_ > 0.0f) {
        int fill_w = static_cast<int>(w * progress_);
        painter.fill_rect(x, y, fill_w, h, color_);
    }
}

void ProgressBar::measure(int& mw, int& mh) {
    mw = 100; // Default width
    mh = 20;  // Default height
}

bool ProgressBar::on_touch(int tx, int ty, bool down) {
     bool inside = (tx >= x && tx < x + w && ty >= y && ty < y + h);
     if (inside && down) {
         float p = (float)(tx - x) / (float)w;
         set_progress(p);
         return true;
     }
     return false;
}

} // namespace Izo
