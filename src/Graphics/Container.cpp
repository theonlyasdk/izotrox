// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Container.hpp"
#include "../Input/Input.hpp"

namespace Izo {

void Container::add_child(std::shared_ptr<Widget> child) {
    m_children.push_back(child);
    Widget::invalidate();
}

void Container::draw_content(Painter& painter) {
    painter.set_clip(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h);
    for (auto& child : m_children) {
        if (child->visible())
            child->draw(painter);
    }
    painter.reset_clip();
}

bool Container::on_touch(int tx, int ty, bool down, bool captured) {
    if (m_captured_child) {
        m_captured_child->on_touch(tx, ty, down, true);
        if (!down) {
            m_captured_child = nullptr;
        }
        return true;
    } 
    
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        auto& child = *it;
        if (!child->visible()) continue;
        
        bool inside = child->bounds().contains(tx, ty);
        if (inside) {
            if (child->on_touch(tx, ty, down, false)) {
                if (down) m_captured_child = child;
                return true;
            }
        } else {
            // Call on_touch with inside=false to allow widgets to handle focus loss on click-outside
            // But only if it's a press event (down=true)
            if (down) {
                child->on_touch(tx, ty, down, false);
            }
        }
    }
    
    return false;
}

bool Container::on_key(int key) {
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

void Container::invalidate() {
    Widget::invalidate();
    for (auto& child : m_children) {
        child->invalidate();
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
    invalidate();
}

} // namespace Izo
