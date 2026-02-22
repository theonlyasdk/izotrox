

#include "View.hpp"
#include "UI/Widgets/Container.hpp"
#include "Input/Input.hpp"

#include "UI/Widgets/Widget.hpp"

namespace Izo {

View::View(std::unique_ptr<Widget> root) : m_root(std::move(root)) {}
View::~View() = default;

void View::resize(int w, int h) {
    m_width = w;
    m_height = h;
    if (m_root) {
        m_root->set_bounds({0, 0, w, h});
        m_root->invalidate_layout();
    }
}

void View::update() {
    if (!m_root) return;

    auto run_layout_pass = [&]() {
        if (!m_root || !m_root->subtree_layout_dirty()) return;

        m_root->set_bounds({0, 0, m_width, m_height});
        m_root->measure(m_width, m_height);

        if (auto* container = dynamic_cast<Container*>(m_root.get())) {
            container->layout();
        } else {
            m_root->layout();
        }

        m_root->clear_layout_dirty_subtree();
        m_root->invalidate_visual();
    };

    run_layout_pass();
    m_root->update();
    run_layout_pass();
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
        Container* root_container = dynamic_cast<Container*>(m_root.get());
        if (root_container) {
            std::vector<Widget*> focusables;
            root_container->collect_focusable_widgets(focusables);
            for (auto* w : focusables) {
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
        std::vector<Widget*> focusables;
        Container* root_container = dynamic_cast<Container*>(m_root.get());
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

        for (auto* w : focusables) w->set_focused(false);

        int nextIdx = (current_idx + 1) % focusables.size();
        focusables[nextIdx]->set_focused(true);
        return;
    }

    if (m_root) m_root->on_key(key);
}

bool View::has_running_animations() const {
    return m_root ? m_root->has_running_animations() : false;
}

} 
