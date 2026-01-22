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
    
protected:
    int content_height() const override { return m_content_height; }
    
private:
    Orientation m_orientation;
    int m_content_height = 0;
};

} // namespace Izo
