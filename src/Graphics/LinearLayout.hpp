// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "Layout.hpp"

namespace Izo {

enum class Orientation { Horizontal, Vertical };

class LinearLayout : public Layout {
public:
    LinearLayout(Orientation orientation = Orientation::Vertical) : m_orientation(orientation) {}
    void layout_children() override;
    
    // Also override measure
    void measure(int parent_w, int parent_h) override;
    
private:
    Orientation m_orientation;
};

} // namespace Izo
