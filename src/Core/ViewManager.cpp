#include "ViewManager.hpp"
#include "UI/View/View.hpp"
#include "Graphics/Painter.hpp"
#include <memory>
#include "Core/Application.hpp"
#include "ThemeDB.hpp"
#include "Input/Input.hpp"
#include "Graphics/Dialog.hpp"

namespace Izo {

ViewManager& ViewManager::the() {
    static ViewManager instance;
    return instance;
}

ViewManager::ViewManager() = default;
ViewManager::~ViewManager() = default;

void ViewManager::setup_transition(ViewTransition transition, bool is_pop) {
    int transition_duration = ThemeDB::the().get<int>("System", "ViewTransitionDuration", 500);
    Easing transition_easing = ThemeDB::the().get<Easing>("System", "ViewTransitionEasing", Easing::EaseOutQuart);

    if (is_pop) {
        if (!m_stack.empty()) {
            m_outgoing_view = std::move(m_stack.back());
            m_stack.pop_back();
        }
    } else {
        m_outgoing_view = nullptr;
    }

    m_current_transition = transition;
    m_animating = true;
    m_is_pop = is_pop;
    m_transition_anim.snap_to(0.0f);
    m_transition_anim.set_target(1.0f, transition_duration, transition_easing);
}

void ViewManager::push(std::unique_ptr<View> view, ViewTransition transition) {
    if (transition == ViewTransition::ThemeDefault) {
        transition = ThemeDB::the().get<ViewTransition>("System", "ViewTransition", ViewTransition::PushLeft);
    }

    // Queue the operation if we're currently processing one
    if (m_processing_operation) {
        m_pending_operations.push_back({OperationType::Push, std::move(view), transition});
        return;
    }

    m_processing_operation = true;
    view->resize(m_width, m_height);

    if (!m_stack.empty() && transition != ViewTransition::None) {
        setup_transition(transition, false);
    }

    m_stack.push_back(std::move(view));
    m_processing_operation = false;
}

void ViewManager::pop(ViewTransition transition) {
    // Calculate effective stack size considering pending pops
    size_t pending_pops = 0;
    for (const auto& op : m_pending_operations) {
        if (op.type == OperationType::Pop) pending_pops++;
    }
    
    size_t effective_stack_size = m_stack.size() - pending_pops;
    
    // Prevent popping if it would leave the stack empty or invalid
    if (effective_stack_size <= 1) {
        return;
    }

    if (transition == ViewTransition::ThemeDefault) {
        transition = ThemeDB::the().get<ViewTransition>("System", "ViewTransition", ViewTransition::PushLeft);
    }

    // Queue the operation if we're currently processing one
    if (m_processing_operation) {
        m_pending_operations.push_back({OperationType::Pop, nullptr, transition});
        return;
    }

    m_processing_operation = true;

    if (transition != ViewTransition::None) {
        setup_transition(transition, true);
    } else {
        m_stack.pop_back();
    }

    m_processing_operation = false;
}

void ViewManager::process_pending_operations() {
    if (m_pending_operations.empty() || m_processing_operation) {
        return;
    }

    PendingOperation pending = std::move(m_pending_operations.front());
    m_pending_operations.pop_front();

    if (pending.type == OperationType::Push) {
        push(std::move(pending.view), pending.transition);
    } else if (pending.type == OperationType::Pop) {
        pop(pending.transition);
    }
}

View* ViewManager::get_active_input_view() {
    // During a pop animation, input should go to the view that will be active
    // (the one below the currently animating outgoing view)
    if (m_animating && m_is_pop && m_stack.size() >= 1) {
        return m_stack.back().get();
    }
    
    // During a push animation, input goes to the new view (already at the top of stack)
    if (m_animating && !m_is_pop && m_stack.size() >= 1) {
        return m_stack.back().get();
    }
    
    // Not animating - return the current top view
    if (!m_stack.empty()) {
        return m_stack.back().get();
    }
    
    return nullptr;
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
        m_dialog->measure(w, h);
    }
}

void ViewManager::update() {
    int scroll = Input::the().scroll_y();

    if (m_dialog) {
        if (scroll != 0) {
            m_dialog->on_scroll(scroll);
        }
        m_dialog->update();
    }

    // m_dialog sometimes might be nullptr after update
    if (m_dialog && !m_dialog->m_dialog_anim.running()) {
        return;
    }

    if (m_animating) {
        float dt = Application::the().delta();
        if (m_transition_anim.update(dt)) {
            // Still animating...
        } else {
            // Animation finished
            m_animating = false;
            m_outgoing_view = nullptr;
            m_current_transition = ViewTransition::None;
            
            // Process any pending operations that were queued during the animation
            process_pending_operations();
        }
    }
    
    // Update the active view (even during animations for parallel input)
    auto active_view = get_active_input_view();
    if (active_view) {
        if (scroll != 0) {
            active_view->on_scroll(scroll);
        }
        active_view->update();
    }
}

void ViewManager::draw(Painter& painter) {
    if (m_stack.empty()) return;

    Color color_win_bg = ThemeDB::the().get<Color>("Colors", "Window.Background", Color(0));

    if (m_animating) {
        float t = m_transition_anim.value();
        float width = (float)m_width;
        float height = (float)m_height;

        View* incoming = m_stack.back().get();
        View* outgoing = nullptr;

        if (m_is_pop) {
            outgoing = m_outgoing_view.get();
        } else if (m_stack.size() >= 2) {
            outgoing = m_stack[m_stack.size() - 2].get();
        }

        auto draw_view = [&](View* v, float tx, float ty, float alpha, bool bg) {
            if (!v) return;
            painter.set_global_alpha(alpha);
            painter.push_translate({(int)tx, (int)ty});
            if (bg) painter.fill_rect({0, 0, m_width, m_height}, color_win_bg);
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
                draw_view(outgoing, -t * width, 0, 1.0f, false);
                draw_view(incoming, (1.0f - t) * width, 0, 1.0f, true);
            } else {
                // Pop: Move Right (Reverse of Push)
                draw_view(incoming, -(1.0f - t) * width, 0, 1.0f, true);
                draw_view(outgoing, t * width, 0, 1.0f, false); 
            }
        } else if (m_current_transition == ViewTransition::SlideRight) {
            if (!m_is_pop) {
                // Push: Move Right
                draw_view(outgoing, t * width, 0, 1.0f, false);
                draw_view(incoming, -(1.0f - t) * width, 0, 1.0f, true);
            } else {
                // Pop: Move Left (Reverse of Push)
                draw_view(incoming, (1.0f - t) * width, 0, 1.0f, true);
                draw_view(outgoing, -t * width, 0, 1.0f, false);
            }
        } else if (m_current_transition == ViewTransition::PushLeft) {
            if (!m_is_pop) {
                // Push: Incoming slides in from Right. Outgoing parallax Left.
                draw_view(outgoing, -t * (width / 4.0f), 0, 1.0f, false);
                draw_shadow(50.0f * t);
                draw_view(incoming, (1.0f - t) * width, 0, 1.0f, true);
            } else {
                // Pop: Outgoing slides out to Right. Incoming parallax from Left.
                draw_view(incoming, -(1.0f - t) * (width / 4.0f), 0, 1.0f, true);
                draw_shadow(50.0f * (1.0f - t));
                draw_view(outgoing, t * width, 0, 1.0f, true); 
            }
        } else if (m_current_transition == ViewTransition::PushRight) {
            if (!m_is_pop) {
               // Push: Incoming slides in from Left. Outgoing slides out Right.
               draw_view(incoming, -(1.0f - t) * (width / 4.0f), 0, 1.0f, true);
               draw_shadow(50.0f * (1.0f - t));
               draw_view(outgoing, t * width, 0, 1.0f, true);
            } else {
               // Pop: Outgoing slides out Left. Incoming slides in from Right.
               draw_view(outgoing, -t * (width / 4.0f), 0, 1.0f, false);
               draw_shadow(50.0f * t);
               draw_view(incoming, (1.0f - t) * width, 0, 1.0f, true);
            }
        } else if (m_current_transition == ViewTransition::PushBottom) {
            if (!m_is_pop) {
                // Push: Slide Up
                draw_view(outgoing, 0, -t * (height / 4.0f), 1.0f, false);
                draw_shadow(100.0f * t);
                draw_view(incoming, 0, (1.0f - t) * height, 1.0f, true);
            } else {
                // Pop: Slide Down
                draw_view(incoming, 0, -(1.0f - t) * (height / 4.0f), 1.0f, true);
                draw_shadow(100.0f * (1.0f - t));
                draw_view(outgoing, 0, t * height, 1.0f, true);
            }
        } else if (m_current_transition == ViewTransition::MaterialUFade) {
            float offset = 50.0f;
            if (!m_is_pop) {
                 // Push: Right to Left (Incoming 50->0)
                 draw_view(outgoing, -offset * t, 0, 1.0f - t, false);
                 draw_view(incoming, offset * (1.0f - t), 0, t, true);
            } else {
                 // Pop: Left to Right (Outgoing 0->50)
                 draw_view(incoming, -offset * (1.0f - t), 0, t, true);
                 draw_view(outgoing, offset * t, 0, 1.0f - t, false);
            }
        }
    } else {
        m_stack.back()->draw(painter);
    }

    if (m_dialog) {
        constexpr uint8_t max_dialog_bg_alpha = 150;
        int dialog_bg_alpha = (uint8_t)(m_dialog->m_dialog_anim.value() * max_dialog_bg_alpha);
        painter.fill_rect({0, 0, m_width, m_height}, Color(0, 0, 0, dialog_bg_alpha));
        m_dialog->draw(painter);
        m_dialog->draw_focus(painter);
    }
}

void ViewManager::on_touch(IntPoint point, bool down) {
    if (m_dialog) {
        m_dialog->on_touch(point, down);
        return;
    }

    // Route input to the active view (even during animations)
    auto active_view = get_active_input_view();
    if (active_view) {
        active_view->on_touch(point, down);
    }
}

void ViewManager::on_key(KeyCode key) {
    // Handle Escape key to pop the view stack
    // This now works during animations too, queueing the operation
    if (key == KeyCode::Escape) {
        if (has_active_dialog()) {
            m_dialog->close();
            return;
        }
        pop();  // Will queue if currently processing an operation
        return;
    }

    if (m_dialog) {
        m_dialog->on_key(key);
        return;
    }

    // Route other keys to the active view (even during animations)
    auto active_view = get_active_input_view();
    if (active_view) {
        active_view->on_key(key);
    }
}

}
