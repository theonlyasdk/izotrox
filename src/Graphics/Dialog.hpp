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

    const bool dim_background_on_open() const { return m_dim_background_on_open; }
    void set_dim_background_on_open(bool enabled) {  m_dim_background_on_open = enabled; }

    void set_auto_layout_centered(bool enabled) { m_auto_layout_centered = enabled; }
    void set_on_dismiss(std::function<void()> callback) { m_on_dismiss = callback; }
protected:
    static int calculate_visual_roundness(int base_roundness, int padding);
    int visual_roundness() const;
    void run_layout_pass();

    bool m_closing = false;
    bool m_dialog_visible = false;
    int m_animation_duration_ms = 300;
    int m_dialog_base_roundness = 12;
    IntRect m_dialog_bounds;
    Animator<float> m_dialog_anim{0.0f};
    std::function<void()> m_on_dismiss;
    Easing m_animation_easing = Easing::EaseOutQuart;
    Color m_dialog_color_bg{40, 40, 40, 240};
    Color m_dialog_color_border{100, 100, 100, 180};
    Color m_dialog_color_shadow{0, 0, 0, 96};
    bool m_auto_layout_centered = false;
    bool m_dim_background_on_open = true;
    int m_last_parent_w = -1;
    int m_last_parent_h = -1;
};

} 
