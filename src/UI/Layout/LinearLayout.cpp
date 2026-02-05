#include "UI/Layout/LinearLayout.hpp"

namespace Izo {

LinearLayout::LinearLayout(Orientation orientation)
    : m_orientation(orientation) {}

void LinearLayout::measure(int parent_w, int parent_h) {
    int w = 0;
    int h = 0;

    for (auto& child : m_children) {
        if (!child->visible()) continue;
        child->measure(parent_w, parent_h); 

        if (m_orientation == Orientation::Vertical) {
            h += child->measured_height() + m_padding_top + m_padding_bottom; 
            if (child->measured_width() > w) w = child->measured_width();
        } else {
            w += child->measured_width() + m_padding_left + m_padding_right;
            if (child->measured_height() > h) h = child->measured_height();
        }
    }

    m_content_height = h + m_padding_top + m_padding_bottom; 

    if (m_width == (int)WidgetSizePolicy::MatchParent) 
        w = parent_w;
    else if (m_width > 0) 
        w = m_width;

    if (m_height == (int)WidgetSizePolicy::MatchParent) 
        h = parent_h;
    else if (m_height > 0) 
        h = m_height;

    m_measured_size = {0, 0, w, h};
}

void LinearLayout::layout_children() {
    int cur_x = m_bounds.x + m_padding_left; 
    int cur_y = m_bounds.y + m_padding_top;

    for (auto& child : m_children) {
        if (!child->visible()) continue;

        int cw = child->measured_width();
        int ch = child->measured_height();

        if (child->width() == (int)WidgetSizePolicy::MatchParent) cw = m_bounds.w - m_padding_left - m_padding_right; 
        if (child->height() == (int)WidgetSizePolicy::MatchParent) ch = m_bounds.h - m_padding_top - m_padding_bottom;

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
