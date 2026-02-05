

#include "View.hpp"
#include "UI/Widgets/Container.hpp"
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
    if (m_root) {
        m_root->draw(painter);
        m_root->draw_focus(painter);
    }
}

void View::on_touch(IntPoint point, bool down) {
    if (down && m_root) {
        // Unfocus widgets if clicked outside their bounds
        auto root_container = std::dynamic_pointer_cast<Container>(m_root);
        if (root_container) {
            std::vector<std::shared_ptr<Widget>> focusables;
            root_container->collect_focusable_widgets(focusables);
            for (auto& w : focusables) {
                if (!w->global_bounds().contains(point)) {
                    w->set_focused(false);
                }
            }
        } else {
             // If root is not container but is widget
             if (!m_root->global_bounds().contains(point)) {
                 m_root->set_focused(false);
             }
        }
    }
    if (m_root) m_root->on_touch(point, down);
}

void View::on_scroll(int y) {
    if (m_root) m_root->on_scroll(y);
}

void View::on_key(KeyCode key) {
    if (key == KeyCode::Tab) { 
        std::vector<std::shared_ptr<Widget>> focusables;
        auto root_container = std::dynamic_pointer_cast<Container>(m_root);
        if (root_container) {
            root_container->collect_focusable_widgets(focusables);
        }

        if (focusables.empty()) return;

        int current_idx = -1;
        for (int i = 0; i < (int)focusables.size(); ++i) {
            if (focusables[i]->focused()) {
                current_idx = i;
                break;
            }
        }

        for (auto& w : focusables) w->set_focused(false);

        int nextIdx = (current_idx + 1) % focusables.size();
        focusables[nextIdx]->set_focused(true);
        return;
    }

    if (m_root) m_root->on_key(key);
}

} 
