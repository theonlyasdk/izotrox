#pragma once
#include "Graphics/View.hpp"
#include "Graphics/Animator.hpp"
#include "Geometry/Point.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace Izo {

enum class ViewTransition {
    None,
    SlideLeft,
    SlideRight
};

class ViewManager {
public:
    static ViewManager& the();
    
    void push(std::shared_ptr<View> view, ViewTransition transition = ViewTransition::SlideLeft);
    void pop(ViewTransition transition = ViewTransition::SlideRight);
    
    void resize(int w, int h);
    void update();
    void draw(Painter& painter);
    void on_touch(IntPoint point, bool down);
    void on_key(KeyCode key);
    
    bool is_animating() const { return m_animating; }
    size_t stack_size() const { return m_stack.size(); }

private:
    ViewManager() = default;
    
    std::vector<std::shared_ptr<View>> m_stack;
    std::shared_ptr<View> m_outgoing_view;
    
    Animator<float> m_transition_anim;
    ViewTransition m_current_transition = ViewTransition::None;
    bool m_animating = false;
    
    int m_width = 0;
    int m_height = 0;
};

} // namespace Izo
