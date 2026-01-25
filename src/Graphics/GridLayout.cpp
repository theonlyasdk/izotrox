

#include "GridLayout.hpp"
#include <algorithm>

namespace Izo {

void GridLayout::layout_children() {
    if (m_children.empty() || m_cols <= 0) return;

    int padding = 10;
    int spacing = 10;

    int available_w = m_bounds.w - 2 * padding;
    int col_w = (available_w - (m_cols - 1) * spacing) / m_cols;

    int cur_x = m_bounds.x + padding;
    int cur_y = m_bounds.y + padding;

    std::vector<int> row_heights;
    int current_h = 0;
    for (size_t i = 0; i < m_children.size(); ++i) {
        if (!m_children[i]->visible()) continue;

        int mh = m_children[i]->measured_height();
        if (mh > current_h) current_h = mh;

        if ((i + 1) % m_cols == 0 || i == m_children.size() - 1) {
            row_heights.push_back(current_h);
            current_h = 0;
        }
    }

    int row_idx = 0;
    int col_idx = 0;
    for (size_t i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        if (!child->visible()) continue;

        int mh = child->measured_height();

        int cell_x = cur_x + col_idx * (col_w + spacing);
        int cell_y = cur_y; 

        int row_h = row_heights[row_idx];
        int y_offset = (row_h - mh) / 2;

        child->set_bounds({cell_x, cell_y + y_offset, col_w, mh});
        child->layout();

        col_idx++;
        if (col_idx >= m_cols) {
            col_idx = 0;
            cur_y += row_h + spacing;
            row_idx++;
        }
    }
}

} 
