

#include "LinearLayout.hpp"

namespace Izo {

void LinearLayout::measure(int parent_w, int parent_h) {
    int w = 0;
    int h = 0;

    for (auto& child : m_children) {
        if (!child->visible()) continue;
        child->measure(parent_w, parent_h); 

        if (m_orientation == Orientation::Vertical) {
            h += child->measured_height() + 10; 
            if (child->measured_width() > w) w = child->measured_width();
        } else {
            w += child->measured_width() + 10;
            if (child->measured_height() > h) h = child->measured_height();
        }
    }

    m_content_height = h + 10; 

    if (m_width == (int)WidgetSizePolicy::MatchParent) w = parent_w;
    else if (m_width > 0) w = m_width;

    if (m_height == (int)WidgetSizePolicy::MatchParent) h = parent_h;
    else if (m_height > 0) h = m_height;

    m_measured_size = {0, 0, w, h};
}

void LinearLayout::layout_children() {
    int cur_x = m_bounds.x + 10; 
    int cur_y = m_bounds.y + 10;

    for (auto& child : m_children) {
        if (!child->visible()) continue;

        int cw = child->measured_width();
        int ch = child->measured_height();

        if (child->width() == (int)WidgetSizePolicy::MatchParent) cw = m_bounds.w - 20; 

        child->set_bounds({cur_x, cur_y, cw, ch});
        child->layout(); 

        if (m_orientation == Orientation::Vertical) {
            cur_y += ch + 10;
        } else {
            cur_x += cw + 10;
        }
    }
}

} 
