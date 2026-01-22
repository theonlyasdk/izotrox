// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "ListItem.hpp"
#include "ListView.hpp"
#include "Core/ThemeDB.hpp"

namespace Izo {

ListItem::ListItem(Orientation orientation) : LinearLayout(orientation) {
    set_width(WidgetSizePolicy::MatchParent);
    set_height(WidgetSizePolicy::WrapContent);
    set_focusable(true);
    set_show_focus_indicator(false); 
}

void ListItem::draw_content(Painter& painter) {
    if (m_selected) {
        painter.fill_rect(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h, ThemeDB::the().color("ListItem.Focus"));
    }
    Container::draw_content(painter);
}

void ListItem::update() {
    Container::update();
}

bool ListItem::on_touch(int tx, int ty, bool down, bool captured) {
    bool handled = Container::on_touch(tx, ty, down, captured);
    
    if (!handled && m_visible) {
         bool inside = m_bounds.contains(tx, ty);
         
         // On release inside, notify parent ListView to select this item
         if (!down && inside && m_touch_started_inside) {
             // Find parent ListView and our index
             if (auto* parent = dynamic_cast<ListView*>(m_parent)) {
                 for (size_t i = 0; i < parent->children().size(); ++i) {
                     if (parent->children()[i].get() == this) {
                         parent->select((int)i);
                         break;
                     }
                 }
             }
         }
         
         handle_focus_logic(inside, down);
         if (inside && down) return true;
    }
    
    return handled;
}

bool ListItem::on_touch_event(int local_x, int local_y, bool down) {
    return true;
}

} // namespace Izo
