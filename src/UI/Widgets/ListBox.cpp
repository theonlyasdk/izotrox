#include "ListBox.hpp"
#include "UI/Widgets/Widget.hpp"
#include "UI/Widgets/ListItem.hpp"
#include "Core/ThemeDB.hpp"
#include "Input/Input.hpp"
#include "Graphics/Painter.hpp"

namespace Izo {

ListBox::ListBox() {
    set_focusable(true);
}

void ListBox::add_item(std::shared_ptr<Widget> item) {
    add_child(item);
}

void ListBox::smooth_scroll_to_index(int index) {
    if (index < 0 || index >= (int)m_children.size()) return;
    auto listitem = m_children[index];

    // IMPORTANT: Use local_bounds() directly (untranslated layout bounds). 
    // DO NOT change this to global_bounds() here.
    float listitem_offset = listitem->local_bounds().y - local_bounds().y;
    float listitem_h = listitem->local_bounds().h;
    float listview_h = local_bounds().h;
    float target_y_pos = m_scroll_y;

    if (listitem_offset < -m_scroll_y) {
        target_y_pos = -listitem_offset;
    } else if (listitem_offset + listitem_h > -m_scroll_y + listview_h) {
        target_y_pos = listview_h - listitem_h - listitem_offset;
    }

    smooth_scroll_to(target_y_pos);
}

void ListBox::select(int index) {
    if (index < 0 || index >= (int)m_children.size()) {
        m_selected_index = -1;
        return;
    }
    m_selected_index = index;
    
    for (size_t i = 0; i < m_children.size(); ++i) {
        auto listItem = std::dynamic_pointer_cast<ListItem>(m_children[i]);
        if (listItem) {
            listItem->set_selected((int)i == m_selected_index);
        }
    }
    
    if (m_on_item_selected && m_selected_index >= 0) {
        m_on_item_selected(m_selected_index);
    }

    smooth_scroll_to_index(m_selected_index);
}

void ListBox::measure(int parent_w, int parent_h) {
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

void ListBox::layout_children() {
    int cur_y = m_bounds.y;
    int idx = 0;
    for (auto& child : m_children) {
        if (!child->visible()) continue;
        child->set_layout_index(idx++);
        int ch = child->measured_height();
        child->set_bounds({m_bounds.x, cur_y, m_bounds.w, ch});
        child->layout();
        cur_y += ch;
    }
}

bool ListBox::on_key(KeyCode key) {
    if (!m_focused) return false;
    
    if (key == KeyCode::Down) {
        if (m_selected_index == -1) {
            for(int i=0; i<(int)m_children.size(); ++i) {
                auto& c = m_children[i];
                if (!c->visible()) continue;
                int item_offset = c->local_bounds().y - m_bounds.y;
                if (item_offset + c->local_bounds().h > -m_scroll_y) {
                    select(i);
                    return true;
                }
            }
            if(!m_children.empty()) select(0);
        } else {
            int next = m_selected_index + 1;
            if (next < (int)m_children.size()) {
                select(next);
            }
        }
        return true;
    } else if (key == KeyCode::Up) {
        if (m_selected_index != -1) {
            int prev = m_selected_index - 1;
            if (prev >= 0) {
                select(prev);
            }
        } else {
             for(int i=0; i<(int)m_children.size(); ++i) {
                auto& c = m_children[i];
                if (!c->visible()) continue;
                int item_offset = c->local_bounds().y - m_bounds.y;
                if (item_offset + c->local_bounds().h > -m_scroll_y) {
                    select(i);
                    return true;
                }
             }
             if(!m_children.empty()) select(0);
        }
        return true;
    }
    
    return false;
}

int ListBox::content_height() const {
    return m_total_content_height;
}

bool ListBox::on_scroll(int y) {
    if (!m_visible) return false;
    
    IntPoint mouse = Input::the().touch_point();
    if (global_bounds().contains(mouse)) {
        int total_content_height = content_height();
        if (total_content_height > m_bounds.h) {
            if (y != 0) {
                m_velocity_y += (float)y * 12.0f;
                m_scrollbar_alpha = 255;
                return true;
            }
        }
    }
    
    return false;
}

void ListBox::draw_content(Painter& painter) {
    Color color_bg = ThemeDB::the().get<Color>("Colors", "ListBox.Background", Color(10));
    Color color_divider = ThemeDB::the().get<Color>("Colors", "ListBox.Divider", Color(200));
    Color color_border = ThemeDB::the().get<Color>("Colors", "ListBox.Border", Color(200));
    Color color_listitem_focus = ThemeDB::the().get<Color>("Colors", "ListItem.Focus", Color(0, 0, 255));
    int widget_roundness = ThemeDB::the().get<int>("Looks", "Widget.Roundness", 6);

    IntRect b = global_bounds();
    painter.fill_rounded_rect(b, widget_roundness, color_bg);

    Layout::draw_content(painter);
    
    int visible_top = b.y;
    int visible_bottom = b.y + b.h;
    painter.push_rounded_clip(b, widget_roundness);

// In ListBox::draw_content
    for (size_t i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        if (!child->visible()) continue;
        
        // Draw selection
        auto item = std::dynamic_pointer_cast<ListItem>(child);
        if (item && item->is_selected()) {
             IntRect cb = child->global_bounds();
             int corners = 0;
             if (i == 0) corners |= Painter::TopLeft | Painter::TopRight;
             if (i == m_children.size() - 1) corners |= Painter::BottomLeft | Painter::BottomRight;
             if (m_children.size() == 1) corners = Painter::AllCorners;
             painter.fill_rounded_rect(cb, widget_roundness, color_listitem_focus, corners);
        }

        IntRect cb = child->global_bounds();
        int item_y = cb.y;
        
        if (item_y + cb.h >= visible_top && item_y <= visible_bottom) {
             // Divider
             if (i < m_children.size() - 1) { // Not last
                 int line_y = item_y + cb.h - 1; 
                 painter.fill_rect({cb.x, line_y, b.w, 1}, color_divider);
             }
        }
    }
    
    painter.pop_clip();
    painter.draw_rounded_rect(b, widget_roundness, color_border);
}

}