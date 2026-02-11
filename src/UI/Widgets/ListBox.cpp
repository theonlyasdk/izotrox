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

void ListBox::add_item(std::unique_ptr<Widget> item) {
    add_child(std::move(item));
}

void ListBox::smooth_scroll_to_index(int index) {
    if (index < 0 || index >= (int)m_children.size()) return;
    const auto& listitem = m_children[index];
    // Use local bounds to avoid compounding scroll offsets.
    float listitem_offset = (float)(listitem->local_bounds().y - local_bounds().y);
    float listitem_h = (float)listitem->local_bounds().h;
    float listview_h = (float)local_bounds().h;
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
        ListItem* listItem = dynamic_cast<ListItem*>(m_children[i].get());
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

    auto first_visible_index = [this]() -> int {
        for (int i = 0; i < (int)m_children.size(); ++i) {
            auto& c = m_children[i];
            if (!c->visible()) continue;
            int item_content_y = c->local_bounds().y - m_bounds.y;
            float view_y = (float)item_content_y + m_scroll_y;
            if (view_y + (float)c->local_bounds().h > 0.0f) {
                return i;
            }
        }
        return m_children.empty() ? -1 : 0;
    };

    auto page_step = [this](int start_index, int direction) -> int {
        if (m_children.empty()) return 0;
        float view_h = (float)local_bounds().h;
        if (view_h <= 0.0f) return 1;

        float accum = 0.0f;
        int step = 0;
        if (direction > 0) {
            for (int i = start_index; i < (int)m_children.size(); ++i) {
                auto& c = m_children[i];
                if (!c->visible()) continue;
                float h = (float)c->local_bounds().h;
                if (step > 0 && accum + h > view_h) break;
                accum += h;
                ++step;
            }
        } else {
            for (int i = start_index; i >= 0; --i) {
                auto& c = m_children[i];
                if (!c->visible()) continue;
                float h = (float)c->local_bounds().h;
                if (step > 0 && accum + h > view_h) break;
                accum += h;
                ++step;
            }
        }

        return step > 0 ? step : 1;
    };
    
    if (key == KeyCode::Down) {
        if (m_selected_index == -1) {
            for(int i=0; i<(int)m_children.size(); ++i) {
                auto& c = m_children[i];
                if (!c->visible()) continue;
                int item_content_y = c->local_bounds().y - m_bounds.y;
                float view_y = (float)item_content_y + m_scroll_y;
                if (view_y + (float)c->local_bounds().h > 0.0f) {
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
                int item_content_y = c->local_bounds().y - m_bounds.y;
                float view_y = (float)item_content_y + m_scroll_y;
                if (view_y + (float)c->local_bounds().h > 0.0f) {
                    select(i);
                    return true;
                }
             }
             if(!m_children.empty()) select(0);
        }
        return true;
    } else if (key == KeyCode::Home) {
        if (m_children.empty()) return false;
        select(0);
        return true;
    } else if (key == KeyCode::End) {
        if (m_children.empty()) return false;
        select((int)m_children.size() - 1);
        return true;
    } else if (key == KeyCode::PageDown) {
        if (m_children.empty()) return false;
        int current = m_selected_index >= 0 ? m_selected_index : first_visible_index();
        if (current < 0) return false;
        int step = page_step(current, 1);
        int target = current + step;
        if (target >= (int)m_children.size()) target = (int)m_children.size() - 1;
        select(target);
        return true;
    } else if (key == KeyCode::PageUp) {
        if (m_children.empty()) return false;
        int current = m_selected_index >= 0 ? m_selected_index : first_visible_index();
        if (current < 0) return false;
        int step = page_step(current, -1);
        int target = current - step;
        if (target < 0) target = 0;
        select(target);
        return true;
    }
    
    return false;
}


bool ListBox::on_scroll(int y) {
    if (!m_visible) return false;
    
    IntPoint mouse = Input::the().touch_point();
    if (global_bounds().contains(mouse)) {
        int total_content_height = content_height();
        if (total_content_height > m_bounds.h) {
            if (y != 0) {
                m_velocity_y += (float)y * 18.0f;
                const float MAX_V = 3000.0f;
                if (m_velocity_y > MAX_V) m_velocity_y = MAX_V;
                if (m_velocity_y < -MAX_V) m_velocity_y = -MAX_V;
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

    IntRect bounds = global_bounds();
    painter.fill_rounded_rect(bounds, widget_roundness, color_bg);

    int visible_top = bounds.y;
    int visible_bottom = bounds.y + bounds.h;
    painter.push_rounded_clip(bounds, widget_roundness);

    // Selection background should render below item content.
    for (size_t i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        if (!child->visible()) continue;
        
        IntRect cb = child->global_bounds();
        int item_y = cb.y;
        if (item_y + cb.h < visible_top || item_y > visible_bottom) continue;

        ListItem* item = dynamic_cast<ListItem*>(child.get());
        if (item && item->is_selected()) {
            int corners = 0;
            if (i == 0) corners |= Painter::TopLeft | Painter::TopRight;
            if (i == m_children.size() - 1) corners |= Painter::BottomLeft | Painter::BottomRight;
            if (m_children.size() == 1) corners = Painter::AllCorners;
            painter.fill_rounded_rect(cb, widget_roundness, color_listitem_focus, corners);
        }
    }

    // Draw children (clipped to rounded bounds).
    int margin = 10;
    int child_visible_top = bounds.y - margin;
    int child_visible_bottom = bounds.y + bounds.h + margin;
    for (auto& child : m_children) {
        if (!child->visible()) continue;

        IntRect child_bounds = child->global_bounds();
        if (child_bounds.y + child_bounds.h >= child_visible_top && child_bounds.y <= child_visible_bottom) {
            child->draw(painter);
        }
    }

    // Dividers on top of items.
    for (size_t i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        if (!child->visible()) continue;
        IntRect cb = child->global_bounds();
        int item_y = cb.y;
        if (item_y + cb.h >= visible_top && item_y <= visible_bottom) {
            if (i < m_children.size() - 1) {
                int line_y = item_y + cb.h - 1; 
                painter.fill_rect({cb.x, line_y, bounds.w, 1}, color_divider);
            }
        }
    }

    // Scrollbar (clipped by rounded corners).
    if (m_scrollbar_alpha > 0) {
        int total_content_height = content_height();
        if (total_content_height > local_bounds().h) {
            float view_ratio = (float)local_bounds().h / total_content_height;
            int max_scroll = -(total_content_height - local_bounds().h);

            float bar_h = (float)local_bounds().h * view_ratio;
            float bar_y;

            if (m_scroll_y > 0) {
                float overscroll = m_scroll_y;
                float size_factor = 1.0f - (overscroll / (float)local_bounds().h);
                if (size_factor < 0.2f) size_factor = 0.2f;
                bar_h *= size_factor;
                bar_y = (float)bounds.y;
            } else if (m_scroll_y < max_scroll) {
                float overscroll = max_scroll - m_scroll_y;
                float size_factor = 1.0f - (overscroll / (float)local_bounds().h);
                if (size_factor < 0.2f) size_factor = 0.2f;
                bar_h *= size_factor;
                bar_y = (float)bounds.y + (float)local_bounds().h - bar_h;
            } else {
                bar_y = (float)bounds.y + (-m_scroll_y * view_ratio);
            }

            Color thumb(150, 150, 150, (uint8_t)m_scrollbar_alpha);
            painter.fill_rect({bounds.x + bounds.w - 6, (int)bar_y, 4, (int)bar_h}, thumb);
        }
    }

    painter.pop_clip();
    painter.draw_rounded_rect(bounds, widget_roundness, color_border);
}

}
