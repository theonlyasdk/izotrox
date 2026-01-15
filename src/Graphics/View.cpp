// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "View.hpp"
#include "Container.hpp"
#include "Input/Input.hpp"

namespace Izo {

View::View(std::shared_ptr<Widget> root) : m_root(root) {}

void View::resize(int w, int h) {
    m_width = w;
    m_height = h;
    if (m_root) {
        m_root->set_bounds({0, 0, w, h});
        m_root->measure(w, h);
        
        Container* container = dynamic_cast<Container*>(m_root.get());
        if (container) {
            container->layout();
        }
    }
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
    if (key == 9) { // Tab
        std::vector<std::shared_ptr<Widget>> focusables;
        auto rootContainer = std::dynamic_pointer_cast<Container>(m_root);
        if (rootContainer) {
            rootContainer->collect_focusable_widgets(focusables);
        }
        
        if (focusables.empty()) return;
        
        int currentIdx = -1;
        for (int i = 0; i < (int)focusables.size(); ++i) {
            if (focusables[i]->focused()) {
                currentIdx = i;
                break;
            }
        }
        
        // Unfocus all
        for (auto& w : focusables) w->set_focused(false);
        
        int nextIdx = (currentIdx + 1) % focusables.size();
        focusables[nextIdx]->set_focused(true);
        return;
    }

    if (m_root) m_root->on_key(key);
}

} // namespace Izo
