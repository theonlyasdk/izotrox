// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "Container.hpp"
#include <vector>
#include <string>
#include <functional>

namespace Izo {

class ListView : public Widget {
public:
    ListView();

    void set_items(const std::vector<std::string>& items);
    
    // Custom item drawer support
    void set_item_drawer(std::function<void(Painter&, int index, int x, int y, int w, int h)> drawer);
    void set_item_count(int count);
    
    // Properties
    void set_item_height(int h) { m_item_height = h; }
    
    // Widget overrides
    void draw_content(Painter& painter) override;
    void update() override;
    bool on_touch_event(int local_x, int local_y, bool down) override;
    void measure(int parent_w, int parent_h) override;

private:
    std::vector<std::string> m_items;
    int m_item_count = 0;
    int m_item_height = 50;
    
    // Physics state
    float m_scroll_y = 0.0f;
    float m_velocity_y = 0.0f;
    bool m_is_dragging = false;
    int m_last_touch_y = 0;
    
    // Scrollbar
    float m_scrollbar_alpha = 0.0f;

    // Selection
    int m_selected_index = -1;
    
    std::function<void(Painter&, int index, int x, int y, int w, int h)> m_drawer;
    
    // Constants from ankrypton
    const float FRICTION = 0.95f;
    const float MIN_VELOCITY = 0.15f;
    const float TENSION = 0.15f;
    const float SCROLLBAR_FADE_SPEED = 1.0f;
};

} // namespace Izo
