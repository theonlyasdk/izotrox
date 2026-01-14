#pragma once
#include "Widget.hpp"
#include "Color.hpp"

namespace Izo {

class ProgressBar : public Widget {
public:
    ProgressBar(float progress = 0.0f);

    void set_progress(float v);
    float get_progress() const;

    void draw(Painter& painter) override;
    void measure(int& mw, int& mh) override;
    bool on_touch(int tx, int ty, bool down) override;

private:
    float progress_;
    Color color_;
    Color bg_color_;
};

} // namespace Izo
