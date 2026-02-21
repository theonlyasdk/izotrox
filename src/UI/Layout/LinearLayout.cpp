#include "UI/Layout/LinearLayout.hpp"

namespace Izo {

LinearLayout::LinearLayout(Orientation orientation)
    : m_orientation(orientation) {
    on_theme_update();
}

void LinearLayout::measure(int parent_w, int parent_h) {
    int w = 0;
    int h = 0;

    int available_w = (m_width == (int)WidgetSizePolicy::MatchParent) ? parent_w : 0;
    int available_h = (m_height == (int)WidgetSizePolicy::MatchParent) ? parent_h : 0;

    // Use parent dims if we don't have constraints yet
    if (available_w <= 0) available_w = parent_w;
    if (available_h <= 0) available_h = parent_h;

    int content_w = available_w - m_padding_left - m_padding_right;
    int content_h = available_h - m_padding_top - m_padding_bottom;

    bool first = true;
    for (auto& child : m_children) {
        if (!child->visible()) continue;
        child->measure(content_w, content_h); 

        if (m_orientation == Orientation::Vertical) {
            if (!first) h += 10; // Spacing
            h += child->measured_height();
            if (child->measured_width() > w) w = child->measured_width();
        } else {
            if (!first) w += 10; // Spacing
            w += child->measured_width();
            if (child->measured_height() > h) h = child->measured_height();
        }
        first = false;
    }

    w += m_padding_left + m_padding_right;
    h += m_padding_top + m_padding_bottom;

    m_content_height = h; 

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
