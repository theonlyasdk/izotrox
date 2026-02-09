#pragma once

#include "UI/View/View.hpp"
#include "Motion/Animator.hpp"
#include "Geometry/Primitives.hpp"

#include <vector>
#include <memory>
#include <deque>

namespace Izo {

enum class ViewTransition {
    None,
    ThemeDefault,
    SlideLeft,  
    SlideRight, 
    PushLeft,   
    PushRight,  
    PushBottom, 
    MaterialUFade
};

class Dialog;

class ViewManager {
public:
    ViewManager(const ViewManager&) = delete;
    ViewManager& operator=(const ViewManager&) = delete;

    static ViewManager& the() {
        static ViewManager instance;
        return instance;
    }

    void push(std::shared_ptr<View> view, ViewTransition transition = ViewTransition::ThemeDefault);
    void pop(ViewTransition transition = ViewTransition::ThemeDefault);

    void open_dialog(std::shared_ptr<Dialog> dialog) { m_dialog = dialog; }
    void dismiss_dialog() { m_dialog.reset(); }
    bool has_active_dialog() const { return m_dialog != nullptr; }
    std::shared_ptr<Dialog> active_dialog() const { return m_dialog; }

    void resize(int w, int h);
    void update();
    void draw(Painter& painter);
    void on_touch(IntPoint point, bool down);
    void on_key(KeyCode key);

    bool is_animating() const { return m_animating; }
    size_t stack_size() const { return m_stack.size(); }

private:
    ViewManager() = default;
    void setup_transition(ViewTransition transition, bool is_pop);
    void process_pending_operations();
    std::shared_ptr<View> get_active_input_view();

    enum class OperationType {
        Push,
        Pop
    };

    struct PendingOperation {
        OperationType type;
        std::shared_ptr<View> view;  // Only used for Push
        ViewTransition transition;
    };

    std::vector<std::shared_ptr<View>> m_stack;
    std::shared_ptr<View> m_outgoing_view;
    std::shared_ptr<Dialog> m_dialog;

    Animator<float> m_transition_anim;
    ViewTransition m_current_transition = ViewTransition::None;
    bool m_animating = false;
    bool m_is_pop = false;

    // Parallel animation support
    std::deque<PendingOperation> m_pending_operations;
    bool m_processing_operation = false;

    int m_width = 0;
    int m_height = 0;
};

} 
