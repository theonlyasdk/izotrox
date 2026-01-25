

#include "ListItem.hpp"
#include "ListView.hpp"
#include "Core/ThemeDB.hpp"

namespace Izo {

ListItem::ListItem(Orientation orientation) : LinearLayout(orientation) {
    set_width(WidgetSizePolicy::MatchParent);
    set_height(WidgetSizePolicy::WrapContent);
    set_focusable(false);
    set_show_focus_indicator(false); 
}

void ListItem::draw_content(Painter& painter) {
    Container::draw_content(painter);
}

void ListItem::update() {
    Container::update();
}

bool ListItem::on_touch(IntPoint point, bool down, bool captured) {
    bool inside = screen_bounds().contains(point);

    if (down && !m_prev_down && inside) {
        m_touch_started_inside_local = true;
    }

    bool handled = Container::on_touch(point, down, captured);

    if (!handled && m_visible) {

         if (!down && inside && m_touch_started_inside_local && !m_gesture_cancelled) {

             if (auto* parent = dynamic_cast<ListView*>(m_parent)) {
                 int idx = layout_index();
                 if (idx >= 0) {
                     parent->select(idx);
                 }
             }
         }

         if (inside && down) handled = true;
    }

    if (!down) {
        m_touch_started_inside_local = false;
    }

    m_prev_down = down;
    return handled;
}

bool ListItem::on_touch_event(IntPoint point, bool down) {
    return true;
}

} 
