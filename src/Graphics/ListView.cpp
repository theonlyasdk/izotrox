#include "ListView.hpp"
#include "ListItem.hpp"
#include "Core/ThemeDB.hpp"

namespace Izo {

ListView::ListView() {
    set_focusable(true);
}

void ListView::add_item(std::shared_ptr<Widget> item) {
    add_child(item);
}

void ListView::select(int index) {
    if (index < 0 || index >= (int)m_children.size()) {
        m_selected_index = -1;
    } else {
        m_selected_index = index;
    }
    
    // Update ListItem selected states
    for (size_t i = 0; i < m_children.size(); ++i) {
        auto listItem = std::dynamic_pointer_cast<ListItem>(m_children[i]);
        if (listItem) {
            listItem->set_selected((int)i == m_selected_index);
        }
    }
    
    if (m_on_item_selected && m_selected_index >= 0) {
        m_on_item_selected(m_selected_index);
    }
    
    // Ensure selected item is visible
    // Ensure selected item is visible
    if (m_selected_index >= 0) {
        auto& child = m_children[m_selected_index];
        int item_top = child->bounds().y;
        int item_bottom = item_top + child->bounds().h;
        int view_top = bounds().y;
        int view_bottom = view_top + bounds().h;
        
        if (item_top < view_top) {
            m_scroll_y += (view_top - item_top);
        } else if (item_bottom > view_bottom) {
            m_scroll_y += (view_bottom - item_bottom);
        }
    }
}

bool ListView::on_key(KeyCode key) {
    if (!m_focused) return false;
    
    if (key == KeyCode::Down) {
        int next = m_selected_index + 1;
        if (next < (int)m_children.size()) {
            select(next);
        }
        return true;
    } else if (key == KeyCode::Up) {
        int prev = m_selected_index - 1;
        if (prev >= 0) {
            select(prev);
        }
        return true;
    }
    
    return false;
}

void ListView::measure(int parent_w, int parent_h) {
    int w = parent_w;
    int h = parent_h;
    
    int content_h = 0;
    for (auto& child : m_children) {
        if (!child->visible()) continue;
        child->measure(parent_w, parent_h);
        content_h += child->measured_height();
    }
    m_total_content_height = content_h;

    if (m_width == (int)WidgetSizePolicy::MatchParent) w = parent_w;
    else if (m_width == (int)WidgetSizePolicy::WrapContent) w = 200;
    else if (m_width > 0) w = m_width;

    if (m_height == (int)WidgetSizePolicy::MatchParent) h = parent_h;
    else if (m_height == (int)WidgetSizePolicy::WrapContent) h = content_h;
    else if (m_height > 0) h = m_height;
    
    m_measured_size = {0, 0, w, h};
}

void ListView::layout_children() {
    int cur_y = m_bounds.y;
    for (auto& child : m_children) {
        if (!child->visible()) continue;
        int ch = child->measured_height();
        child->set_bounds({m_bounds.x, cur_y, m_bounds.w, ch});
        child->layout();
        cur_y += ch;
    }
}

int ListView::content_height() const {
    return m_total_content_height;
}

void ListView::draw_content(Painter& painter) {
    Color bg = ThemeDB::the().color("ListView.Background");
    if (bg.a == 0) bg = ThemeDB::the().color("Window.Background");
    
    IntRect b = bounds();
    painter.fill_rect(b, bg);

    Layout::draw_content(painter);
    
    // Optional: Draw dividers
    Color divColor = ThemeDB::the().color("ListView.Divider");
    // Visible Y range in SCREEN COORDS
    int visible_top = b.y;
    int visible_bottom = b.y + b.h;

    // Use push clip for dividers as well!
    painter.push_clip(b);
    // painter.push_translate({0, (int)m_scroll_y}); // Removed

    for (auto& child : m_children) {
        if (!child->visible()) continue;
        IntRect cb = child->bounds();
        int item_y = cb.y;
        
        // Cull dividers
        if (item_y + cb.h >= visible_top && item_y <= visible_bottom) {
             // Draw relative to child (which is absolute now)
             int line_y = item_y + cb.h - 1; 
             painter.fill_rect({cb.x, line_y, b.w, 1}, divColor);
        }
    }
    
    // painter.pop_translate();
    painter.pop_clip();

    // Draw border
    painter.draw_rect(b, ThemeDB::the().color("ListView.Border"));
}

} // namespace Izo