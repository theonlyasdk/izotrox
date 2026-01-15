// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Container.hpp"
#include "../Input/Input.hpp"

namespace Izo {

void Container::add_child(std::shared_ptr<Widget> child) {
    m_children.push_back(child);
    Widget::invalidate();
}

void Container::draw_content(Painter& painter) {
    for (auto& child : m_children) {
        if (child->is_visible())
            child->draw(painter);
    }
}

bool Container::on_touch(int tx, int ty, bool down) {
    if (m_captured_child) {
        m_captured_child->on_touch(tx, ty, down);
        if (!down) {
            m_captured_child = nullptr;
        }
        return true;
    } 
    
    if (down) {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it)->is_visible() && (*it)->on_touch(tx, ty, down)) {
                m_captured_child = *it;
                return true;
            }
        }
    } else {
         for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it)->is_visible())
                (*it)->on_touch(tx, ty, down);
        }
    }
    
    return false;
}

bool Container::on_key(int key) {
    for (auto& child : m_children) {
        if (child->is_visible()) {
            if (child->on_key(key)) return true;
        }
    }
    return false;
}

void Container::update() {
    for (auto& child : m_children) {
        if (child->is_visible())
            child->update();
    }
}

void Container::invalidate() {
    Widget::invalidate();
    for (auto& child : m_children) {
        child->invalidate();
    }
}

void Container::layout() {
}

} // namespace Izo
