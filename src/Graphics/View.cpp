// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "View.hpp"

namespace Izo {

View::View(std::shared_ptr<Widget> root) : m_root(root) {}

void View::resize(int w, int h) {
    m_width = w;
    m_height = h;
    if (m_root) {
        // Measure first
        m_root->measure(w, h);
        // Then set bounds
        m_root->set_bounds({0, 0, w, h}); // Root fills view?
        // Or root uses its measured size?
        // Usually root layout fills screen.
        m_root->set_bounds({0, 0, w, h});
        
        // If root is layout, it will layout children in set_bounds override?
        // Widget::set_bounds just sets rect.
        // Layout must override set_bounds or we call layout() explicitly?
        // Layout logic should be in measure/layout pass.
        // I should add `layout()` method to Widget interface or cast?
        // Or `set_bounds` triggers layout in Layout class.
        
        // Let's assume Layout overrides set_bounds or we need a layout pass.
        // I should add `virtual void layout()` to Widget (default empty).
        // Check Widget.hpp I just wrote. It DOES NOT have layout().
        // I should add it.
        m_root->layout();
    }
    if (m_root) m_root->invalidate();
}

void View::update() {
    if (m_root) m_root->update();
}

void View::draw(Painter& painter) {
    if (m_root) m_root->draw(painter);
}

void View::on_touch(int x, int y, bool down) {
    if (m_root) m_root->on_touch(x, y, down);
}

void View::on_key(int key) {
    if (m_root) m_root->on_key(key);
}

} // namespace Izo
