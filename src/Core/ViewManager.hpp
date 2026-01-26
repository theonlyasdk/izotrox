#pragma once
#include "Graphics/View.hpp"
#include "Graphics/Animator.hpp"
#include "Geometry/Primitives.hpp"
#include <vector>
#include <memory>
#include <functional>

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

class ViewManager {
public:
    static ViewManager& the();

    void push(std::shared_ptr<View> view, ViewTransition transition = ViewTransition::ThemeDefault);
    void pop(ViewTransition transition = ViewTransition::ThemeDefault);

    void show_dialog(std::shared_ptr<Widget> dialog);
    void dismiss_dialog();
    bool has_active_dialog() const { return m_dialog != nullptr; }

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

    std::vector<std::shared_ptr<View>> m_stack;
    std::shared_ptr<View> m_outgoing_view;
    std::shared_ptr<Widget> m_dialog;

    Animator<float> m_transition_anim;
    ViewTransition m_current_transition = ViewTransition::None;
    bool m_animating = false;
    bool m_is_pop = false;

    int m_width = 0;
    int m_height = 0;
};

} 
