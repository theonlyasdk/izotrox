// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Layout.hpp"

namespace Izo {

void Layout::measure(int parent_w, int parent_h) {
    // Layout measurement involves measuring children
    // Default implementation could just measure children?
    // Subclasses like LinearLayout needs to implement this properly.
    
    // For now, let's just set measured size to parent (MatchParent behavior default)
    // or wrap content?
    // We'll leave it to subclasses to implement proper measure.
    // But we need base implementation.
    
    m_measured_size = {0, 0, 0, 0};
    if (m_width == MatchParent) m_measured_size.w = parent_w;
    if (m_height == MatchParent) m_measured_size.h = parent_h;
}

void Layout::layout() {
    layout_children();
}

} // namespace Izo