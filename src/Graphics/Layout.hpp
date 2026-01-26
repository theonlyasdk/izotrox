#pragma once

#include "Container.hpp"

namespace Izo {

class Layout : public Container {
public:
    void layout() override;
    virtual void layout_children() = 0;

    void measure(int parent_w, int parent_h) override;
    void update() override;
    void draw_content(Painter& painter) override;
    void draw_focus(Painter& painter) override;
    bool on_touch(IntPoint point, bool down, bool captured = false) override;
    bool on_touch_event(IntPoint point, bool down) override;
    bool on_scroll(int y) override;
    bool is_scrollable() const override { return true; }
    IntPoint content_scroll_offset() const override { return {0, (int)m_scroll_y}; }

    void smooth_scroll_to(int target_y);
    virtual void smooth_scroll_to_index(int index);

protected:
    virtual int content_height() const = 0;

    float m_scroll_y = 0.0f;
    float m_velocity_y = 0.0f;
    bool m_is_dragging = false;
    int m_last_touch_y = 0;

    bool m_auto_scrolling = false;
    float m_auto_scroll_target = 0.0f;

    int m_initial_touch_x = 0;
    int m_initial_touch_y = 0;
    bool m_potential_swipe = false;
    bool m_has_intercepted = false;

    float m_scrollbar_alpha = 0.0f;

    const float FRICTION = 0.95f;
    const float MIN_VELOCITY = 0.15f;
    const float TENSION = 0.15f;
    const float SCROLLBAR_FADE_SPEED = 1.0f;
    const float AUTO_SCROLL_SPEED = 10.0f; 
};

} 
