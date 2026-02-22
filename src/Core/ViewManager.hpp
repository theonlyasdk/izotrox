#pragma once

#include "Motion/Animator.hpp"
#include "Geometry/Primitives.hpp"
#include "Input/KeyCode.hpp"
#include "UI/View/View.hpp"
#include "Graphics/Dialog.hpp"

#include <vector>
#include <memory>
#include <deque>
#include <span>

namespace Izo {

class Painter;
class View;
class Dialog;

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


class ViewManager {
public:
    ViewManager(const ViewManager&) = delete;
    ViewManager& operator=(const ViewManager&) = delete;

    static ViewManager& the();
    ~ViewManager();


    void push(std::unique_ptr<View> view, ViewTransition transition = ViewTransition::ThemeDefault);
    void pop(ViewTransition transition = ViewTransition::ThemeDefault);

    void open_dialog(std::unique_ptr<Dialog> dialog);
    void dismiss_dialog();
    bool has_active_dialog() const { return m_dialog != nullptr; }
    Dialog* active_dialog() const { return m_dialog.get(); }

    void resize(int w, int h);
    void update();
    void draw(Painter& painter);
    void on_touch(IntPoint point, bool down);
    void on_key(KeyCode key);
    void invalidate_rect(const IntRect& rect);
    void invalidate_full();
    bool has_dirty() const;
    std::vector<IntRect> consume_dirty_rects();
    bool needs_redraw() const;

    bool is_animating() const { return m_animating; }
    size_t stack_size() const { return m_stack.size(); }

private:
    ViewManager();
    void setup_transition(ViewTransition transition, bool is_pop);
    void process_pending_operations();
    View* get_active_input_view();

    enum class OperationType {
        Push,
        Pop
    };

    struct PendingOperation {
        OperationType type;
        std::unique_ptr<View> view;  // Only used for Push
        ViewTransition transition;
    };

    std::vector<std::unique_ptr<View>> m_stack;
    std::unique_ptr<View> m_outgoing_view;
    std::unique_ptr<Dialog> m_dialog;

    Animator<float> m_transition_anim;
    ViewTransition m_current_transition = ViewTransition::None;
    bool m_animating = false;
    bool m_is_pop = false;

    // Parallel animation support
    std::deque<PendingOperation> m_pending_operations;
    bool m_processing_operation = false;

    std::vector<IntRect> m_dirty_rects;
    bool m_full_redraw_needed = true;

    int m_width = 0;
    int m_height = 0;
};

} 
