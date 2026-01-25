#include "ViewManager.hpp"
#include "Application.hpp"
#include "Input/Input.hpp"

namespace Izo {

ViewManager& ViewManager::the() {
    static ViewManager instance;
    return instance;
}

void ViewManager::push(std::shared_ptr<View> view, ViewTransition transition) {
    if (m_animating) return;
    
    view->resize(m_width, m_height);
    
    if (!m_stack.empty() && transition != ViewTransition::None) {
        m_outgoing_view = m_stack.back();
        m_current_transition = transition;
        m_animating = true;
        m_transition_anim.snap_to(0.0f);
        m_transition_anim.set_target(1.0f, 500, Easing::EaseOutCubic);
    }
    
    m_stack.push_back(view);
}

void ViewManager::pop(ViewTransition transition) {
    if (m_animating) return;
    if (m_stack.size() <= 1) return;
    
    if (transition != ViewTransition::None) {
        m_outgoing_view = m_stack.back();
        m_current_transition = transition;
        m_animating = true;
        m_transition_anim.snap_to(0.0f);
        m_transition_anim.set_target(1.0f, 500, Easing::EaseOutCubic);
    }
    
    m_stack.pop_back();
}

void ViewManager::resize(int w, int h) {
    m_width = w;
    m_height = h;
    for (auto& view : m_stack) {
        view->resize(w, h);
    }
    if (m_outgoing_view) {
        m_outgoing_view->resize(w, h);
    }
}

void ViewManager::update() {
    if (m_animating) {
        m_transition_anim.update(Application::the().delta());
        if (m_transition_anim.value() >= 1.0f) {
            m_animating = false;
            m_outgoing_view = nullptr;
            m_current_transition = ViewTransition::None;
        }
    }
    
    if (!m_stack.empty()) {
        int scroll = Input::the().scroll_y();
        if (scroll != 0) {
            m_stack.back()->on_scroll(scroll);
        }
        m_stack.back()->update();
    }
}

void ViewManager::draw(Painter& painter) {
    if (m_stack.empty()) return;
    
    float t = m_transition_anim.value();
    
    if (m_animating && m_outgoing_view) {
        int offset = (int)(t * m_width);
        
        if (m_current_transition == ViewTransition::SlideLeft) {
            painter.push_translate({-offset, 0});
            m_outgoing_view->draw(painter);
            painter.pop_translate();
            
            painter.push_translate({m_width - offset, 0});
            m_stack.back()->draw(painter);
            painter.pop_translate();
        } else if (m_current_transition == ViewTransition::SlideRight) {
            painter.push_translate({offset, 0});
            m_outgoing_view->draw(painter);
            painter.pop_translate();
            
            painter.push_translate({-m_width + offset, 0});
            m_stack.back()->draw(painter);
            painter.pop_translate();
        }
    } else {
        m_stack.back()->draw(painter);
    }
}

void ViewManager::on_touch(IntPoint point, bool down) {
    if (m_animating) return;
    if (!m_stack.empty()) {
        m_stack.back()->on_touch(point, down);
    }
}

void ViewManager::on_key(KeyCode key) {
    if (m_animating) return;
    if (!m_stack.empty()) {
        m_stack.back()->on_key(key);
    }
}

} // namespace Izo
