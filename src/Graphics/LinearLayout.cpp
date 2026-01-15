// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "LinearLayout.hpp"

namespace Izo {

void LinearLayout::measure(int parent_w, int parent_h) {
    int w = 0;
    int h = 0;
    
    for (auto& child : m_children) {
        if (!child->is_visible()) continue;
        child->measure(parent_w, parent_h); // Pass available space
        
        if (m_orientation == Orientation::Vertical) {
            h += child->measured_height() + 10; // Spacing
            if (child->measured_width() > w) w = child->measured_width();
        } else {
            w += child->measured_width() + 10;
            if (child->measured_height() > h) h = child->measured_height();
        }
    }
    
    // Adjust for padding
    // ...
    
    // Policy check
    if (m_width == MatchParent) w = parent_w;
    if (m_height == MatchParent) h = parent_h;
    
    m_measured_size = {0, 0, w, h};
}

void LinearLayout::layout_children() {
    int cur_x = m_bounds.x + 10; // padding
    int cur_y = m_bounds.y + 10;
    
    for (auto& child : m_children) {
        if (!child->is_visible()) continue;
        
        int cw = child->measured_width();
        int ch = child->measured_height();
        
        // Handle MatchParent children?
        if (child->width() == MatchParent) cw = m_bounds.w - 20; // minus padding
        // if (child->height() == MatchParent) ch = ...; // Need residual space logic
        
        child->set_bounds({cur_x, cur_y, cw, ch});
        child->layout(); // Recurse
        
        if (m_orientation == Orientation::Vertical) {
            cur_y += ch + 10;
        } else {
            cur_x += cw + 10;
        }
    }
}

} // namespace Izo
