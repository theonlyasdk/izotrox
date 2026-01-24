// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Container.hpp"
#include "Input/Input.hpp"

namespace Izo {

void Container::add_child(std::shared_ptr<Widget> child) {
    child->set_parent(this);
    m_children.push_back(child);
}

void Container::draw_content(Painter& painter) {
    painter.push_clip(m_bounds);
    for (auto& child : m_children) {
        if (child->visible())
            child->draw(painter);
    }
    painter.pop_clip();
}

bool Container::on_touch(IntPoint point, bool down, bool captured) {
    if (m_captured_child) {
        m_captured_child->on_touch(point, down, true);
        if (!down) {
            m_captured_child = nullptr;
        }
        return true;
    } 
    
    std::shared_ptr<Widget> target = nullptr;
    bool handled = false;

    // Pass 1: Find target
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        auto& child = *it;
        if (!child->visible()) continue;
        
        bool inside = child->bounds().contains(point);
        if (inside) {
            // Tentatively try to handle
            if (child->on_touch(point, down, false)) {
                target = child;
                handled = true;
                if (down) m_captured_child = child;
                break; // Found our target (top-most)
            }
        }
    }

    // Pass 2: Focus Management
    if (handled && down) {
        for (auto& child : m_children) {
            if (child != target && child->visible()) {
                // Send inside=false by passing coordinates far away
                child->on_touch(point, down, false); 
            }
        }
    }
    
    return handled;
}

bool Container::on_key(KeyCode key) {
    for (auto& child : m_children) {
        if (child->visible()) {
            if (child->on_key(key)) return true;
        }
    }
    return false;
}

void Container::update() {
    Widget::update();
    for (auto& child : m_children) {
        if (child->visible())
            child->update();
    }
}

void Container::collect_focusable_widgets(std::vector<std::shared_ptr<Widget>>& out_list) {
    for (auto& child : m_children) {
        if (!child->visible()) continue;
        if (child->focusable()) out_list.push_back(child);
        
        Container* container = dynamic_cast<Container*>(child.get());
        if (container) {
            container->collect_focusable_widgets(out_list);
        }
    }
}

void Container::layout() {
}

void Container::draw_focus(Painter& painter) {
    for (auto& child : m_children) {
        if (child->visible())
            child->draw_focus(painter);
    }
    Widget::draw_focus(painter);
}

} // namespace Izo
