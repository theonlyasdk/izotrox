// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "LinearLayout.hpp"

namespace Izo {

void LinearLayout::measure(int parent_w, int parent_h) {
    int w = 0;
    int h = 0;
    
    for (auto& child : m_children) {
        if (!child->visible()) continue;
        child->measure(parent_w, parent_h); // Pass available space
        
        if (m_orientation == Orientation::Vertical) {
            h += child->measured_height() + 10; // Spacing
            if (child->measured_width() > w) w = child->measured_width();
        } else {
            w += child->measured_width() + 10;
            if (child->measured_height() > h) h = child->measured_height();
        }
    }
    
    // Store content height for scrolling
    m_content_height = h + 10; // Add padding
    
    // Policy check
    if (m_width == (int)WidgetSizePolicy::MatchParent) w = parent_w;
    else if (m_width > 0) w = m_width;
    // else it stays as content width (w already accumulated or determined in loop)

    if (m_height == (int)WidgetSizePolicy::MatchParent) h = parent_h;
    else if (m_height > 0) h = m_height;
    
    m_measured_size = {0, 0, w, h};
}

void LinearLayout::layout_children() {
    int cur_x = m_bounds.x + 10; // padding
    int cur_y = m_bounds.y + 10;
    
    for (auto& child : m_children) {
        if (!child->visible()) continue;
        
        int cw = child->measured_width();
        int ch = child->measured_height();
        
        // Handle MatchParent children?
        if (child->width() == (int)WidgetSizePolicy::MatchParent) cw = m_bounds.w - 20; // minus padding
        // if (child->height() == (int)WidgetSizePolicy::MatchParent) ch = ...; // Need residual space logic
        
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
