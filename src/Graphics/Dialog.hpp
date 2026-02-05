#pragma once

#include "UI/Widgets/Widget.hpp"
#include "Graphics/Font.hpp"
#include "Motion/Animator.hpp"
#include "Geometry/Primitives.hpp"

#include <functional>

namespace Izo {

class Dialog : public Widget {
public:
    Dialog();
    virtual ~Dialog() = default;

    void draw_content(Painter& painter) override;
    virtual void draw_dialog_content(Painter& painter) { (void)painter; }
    void update() override;
    
    void show_dialog();
    void hide_dialog();
    bool is_dialog_visible() const { return m_dialog_visible; }

    void set_dialog_bounds(const IntRect& bounds) { m_dialog_bounds = bounds; }
    IntRect dialog_bounds() const { return m_dialog_bounds; }
    
    void set_on_dismiss(std::function<void()> callback) { m_on_dismiss = callback; }

protected:
    IntRect m_dialog_bounds;
    bool m_dialog_visible = false;
    Animator<float> m_dialog_anim{0.0f};
    
    std::function<void()> m_on_dismiss;
};

} 
