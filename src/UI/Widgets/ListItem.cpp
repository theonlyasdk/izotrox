

#include "UI/Widgets/ListItem.hpp"
#include "ListBox.hpp"
#include "Core/ThemeDB.hpp"

namespace Izo {

ListItem::ListItem(Orientation orientation) : LinearLayout(orientation) {
    set_width(WidgetSizePolicy::MatchParent);
    set_height(WidgetSizePolicy::WrapContent);
    set_focusable(false);
    set_show_focus_indicator(false);
    set_padding(5);
}

void ListItem::draw_content(Painter& painter) {
    Container::draw_content(painter);
}

void ListItem::update() {
    Container::update();
}

void ListItem::set_selected(bool sel) {
    if (m_selected == sel) return;
    m_selected = sel;
    invalidate_visual();
}

bool ListItem::on_touch(IntPoint point, bool down, bool captured) {
    bool inside = global_bounds().contains(point);

    if (down && !m_prev_down && inside) {
        m_touch_started_inside_local = true;
    }

    bool handled = Container::on_touch(point, down, captured);

    if (!handled && m_visible) {

         if (!down && inside && m_touch_started_inside_local && !m_gesture_cancelled) {

             if (auto* parent = dynamic_cast<ListBox*>(m_parent)) {
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
