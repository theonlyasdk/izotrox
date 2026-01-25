#include "ViewManager.hpp"
#include "Application.hpp"
#include "ThemeDB.hpp"
#include "Input/Input.hpp"

namespace Izo {

ViewManager& ViewManager::the() {
    static ViewManager instance;
    return instance;
}

void ViewManager::setup_transition(ViewTransition transition, bool is_pop) {
    m_outgoing_view = m_stack.back();
    m_current_transition = transition;
    m_animating = true;
    m_is_pop = is_pop;
    m_transition_anim.snap_to(0.0f);
    m_transition_anim.set_target(1.0f, ThemeDB::the().int_value("ViewTransitionDuration", 500), ThemeDB::the().enum_value<Easing>("ViewTransitionEasing", Easing::EaseOutQuart));
}

void ViewManager::push(std::shared_ptr<View> view, ViewTransition transition) {
    if (transition == ViewTransition::ThemeTransition) {
        transition = ThemeDB::the().enum_value<ViewTransition>("ViewTransition", ViewTransition::PushLeft);
    }

    if (m_animating) return;

    view->resize(m_width, m_height);

    if (!m_stack.empty() && transition != ViewTransition::None) {
        setup_transition(transition, false);
    }

    m_stack.push_back(view);
}

void ViewManager::pop(ViewTransition transition) {
    if (m_animating || m_stack.size() <= 1) return;

    if (transition == ViewTransition::ThemeTransition) {
        transition = ThemeDB::the().enum_value<ViewTransition>("ViewTransition", ViewTransition::PushLeft);
    }

    if (transition != ViewTransition::None) {
        setup_transition(transition, true);
    }

    m_stack.pop_back();
}

void ViewManager::show_dialog(std::shared_ptr<Widget> dialog) {
    m_dialog = dialog;
}

void ViewManager::dismiss_dialog() {
    m_dialog = nullptr;
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
    if (m_dialog) {
        m_dialog->measure(w, h); // Adjust dialog if needed
    }
}

void ViewManager::update() {
    if (m_dialog) {
        m_dialog->update();
        return; // Dialog blocks view updates
    }

    if (m_animating) {
        float dt = Application::the().delta();
        if (m_transition_anim.update(dt)) {
            // Animating...
        } else {
            m_animating = false;
            m_outgoing_view = nullptr;
            m_current_transition = ViewTransition::None; // Reset transition type
        }
    } else if (!m_stack.empty()) { // Only update current view if not animating and no dialog
        int scroll = Input::the().scroll_y();
        if (scroll != 0) {
            m_stack.back()->on_scroll(scroll);
        }
        m_stack.back()->update();
    }
}

void ViewManager::draw(Painter& painter) {
    if (m_stack.empty()) return;

    if (m_animating && m_outgoing_view) {
        float t = m_transition_anim.value();
        float width = (float)m_width;
        float height = (float)m_height;

        auto draw_view = [&](std::shared_ptr<View> v, float tx, float ty, float alpha, bool bg) {
            if (!v) return;
            painter.set_global_alpha(alpha);
            painter.push_translate({(int)tx, (int)ty});
            if (bg) painter.fill_rect({0, 0, m_width, m_height}, ThemeDB::the().color("Window.Background"));
            v->draw(painter);
            painter.pop_translate();
            painter.set_global_alpha(1.0f);
        };

        auto draw_shadow = [&](float opacity) {
            painter.fill_rect({0, 0, m_width, m_height}, Color(0, 0, 0, (uint8_t)opacity));
        };

        if (m_current_transition == ViewTransition::SlideLeft) {
            if (!m_is_pop) {
                // Push: Move Left
                draw_view(m_outgoing_view, -t * width, 0, 1.0f, false);
                draw_view(m_stack.back(), (1.0f - t) * width, 0, 1.0f, true);
            } else {
                // Pop: Move Right (Reverse of Push)
                draw_view(m_stack.back(), -(1.0f - t) * width, 0, 1.0f, true);
                draw_view(m_outgoing_view, t * width, 0, 1.0f, false); 
            }
        } else if (m_current_transition == ViewTransition::SlideRight) {
            if (!m_is_pop) {
                // Push: Move Right
                draw_view(m_outgoing_view, t * width, 0, 1.0f, false);
                draw_view(m_stack.back(), -(1.0f - t) * width, 0, 1.0f, true);
            } else {
                // Pop: Move Left (Reverse of Push)
                draw_view(m_stack.back(), (1.0f - t) * width, 0, 1.0f, true);
                draw_view(m_outgoing_view, -t * width, 0, 1.0f, false);
            }
        } else if (m_current_transition == ViewTransition::PushLeft) {
            if (!m_is_pop) {
                // Push: Incoming slides in from Right. Outgoing parallax Left.
                draw_view(m_outgoing_view, -t * (width / 4.0f), 0, 1.0f, false);
                draw_shadow(50.0f * t);
                draw_view(m_stack.back(), (1.0f - t) * width, 0, 1.0f, true);
            } else {
                // Pop: Outgoing slides out to Right. Incoming parallax from Left.
                draw_view(m_stack.back(), -(1.0f - t) * (width / 4.0f), 0, 1.0f, true);
                draw_shadow(50.0f * (1.0f - t));
                draw_view(m_outgoing_view, t * width, 0, 1.0f, true); 
            }
        } else if (m_current_transition == ViewTransition::PushRight) {
            if (!m_is_pop) {
               // Push: Incoming slides in from Left. Outgoing slides out Right. (Like iOS Pop?)
               draw_view(m_stack.back(), -(1.0f - t) * (width / 4.0f), 0, 1.0f, true);
               draw_shadow(50.0f * (1.0f - t));
               draw_view(m_outgoing_view, t * width, 0, 1.0f, true);
            } else {
               // Pop: Outgoing slides out Left. Incoming slides in from Right.
               draw_view(m_outgoing_view, -t * (width / 4.0f), 0, 1.0f, false);
               draw_shadow(50.0f * t);
               draw_view(m_stack.back(), (1.0f - t) * width, 0, 1.0f, true);
            }
        } else if (m_current_transition == ViewTransition::PushBottom) {
            if (!m_is_pop) {
                // Push: Slide Up
                draw_view(m_outgoing_view, 0, -t * (height / 4.0f), 1.0f, false);
                draw_shadow(100.0f * t);
                draw_view(m_stack.back(), 0, (1.0f - t) * height, 1.0f, true);
            } else {
                // Pop: Slide Down
                draw_view(m_stack.back(), 0, -(1.0f - t) * (height / 4.0f), 1.0f, true);
                draw_shadow(100.0f * (1.0f - t));
                draw_view(m_outgoing_view, 0, t * height, 1.0f, true);
            }
        } else if (m_current_transition == ViewTransition::MaterialUFade) {
            float offset = 50.0f;
            if (!m_is_pop) {
                 // Push: Right to Left (Incoming 50->0)
                 draw_view(m_outgoing_view, -offset * t, 0, 1.0f - t, false);
                 draw_view(m_stack.back(), offset * (1.0f - t), 0, t, true);
            } else {
                 // Pop: Left to Right (Outgoing 0->50)
                 draw_view(m_stack.back(), -offset * (1.0f - t), 0, t, true);
                 draw_view(m_outgoing_view, offset * t, 0, 1.0f - t, false);
            }
        }
    } else {
        m_stack.back()->draw(painter);
    }

    if (m_dialog) {
        painter.fill_rect({0, 0, m_width, m_height}, Color(0, 0, 0, 100)); // Dimming
        m_dialog->draw(painter);
        m_dialog->draw_focus(painter);
    }
}

void ViewManager::on_touch(IntPoint point, bool down) {
    if (m_dialog) {
        m_dialog->on_touch(point, down);
        return;
    }

    if (m_animating) return;
    if (!m_stack.empty()) {
        m_stack.back()->on_touch(point, down);
    }
}

void ViewManager::on_key(KeyCode key) {
    if (m_dialog) {
        m_dialog->on_key(key);
        return;
    }

    if (m_animating) return;
    if (!m_stack.empty()) {
        m_stack.back()->on_key(key);
    }
}

}
