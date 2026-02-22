#pragma once

#include "UI/Layout/LinearLayout.hpp"

#include <functional>

namespace Izo {

class ViewManager;

class Dialog : public LinearLayout {
public:
    friend class ViewManager;
    Dialog();
    virtual ~Dialog() = default;

    void draw_content(Painter& painter) override;
    void update() override;
    void on_theme_update() override;
    
    virtual void draw_dialog_content(Painter& painter) {};
    virtual void open();
    virtual void close();

    bool visible() const { return m_dialog_visible; }
    void set_dialog_bounds(const IntRect& bounds) { m_dialog_bounds = bounds; }
    IntRect dialog_bounds() const { return m_dialog_bounds; }
    void set_on_dismiss(std::function<void()> callback) { m_on_dismiss = callback; }

protected:
    bool m_closing = false;
    bool m_dialog_visible = false;
    int m_animation_duration_ms = 300;
    IntRect m_dialog_bounds;
    Animator<float> m_dialog_anim{0.0f};
    std::function<void()> m_on_dismiss;
    Easing m_animation_easing = Easing::EaseOutQuart;
};

} 
