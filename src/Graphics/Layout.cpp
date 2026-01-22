// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Layout.hpp"
#include "Input/Input.hpp"
#include "Core/ThemeDB.hpp"
#include <cmath>

namespace Izo {

void Layout::measure(int parent_w, int parent_h) { 
    m_measured_size = {0, 0, 0, 0};
    if (m_width == (int)WidgetSizePolicy::MatchParent) m_measured_size.w = parent_w;
    if (m_height == (int)WidgetSizePolicy::MatchParent) m_measured_size.h = parent_h;
}

void Layout::layout() {
    layout_children();
}

void Layout::update() {
    int total_content_height = content_height();
    int max_scroll = (total_content_height > m_bounds.h) ? -(total_content_height - m_bounds.h) : 0;

    int scroll_delta = Input::the().scroll_y();
    if (scroll_delta != 0) {
        m_velocity_y += (float)scroll_delta * 12.0f;
        m_scrollbar_alpha = 255;
    }

    if (!m_is_dragging) {
        if (std::abs(m_velocity_y) > 0.001f || m_scroll_y > 0 || m_scroll_y < max_scroll) {
            m_scroll_y += m_velocity_y;

            if (m_scroll_y > 0) {
                m_velocity_y = (0 - m_scroll_y) * TENSION;
                m_scrollbar_alpha = 255;
                if (std::abs(m_scroll_y) < 0.5f) { m_scroll_y = 0; m_velocity_y = 0; }
            } else if (m_scroll_y < max_scroll) {
                m_velocity_y = (max_scroll - m_scroll_y) * TENSION;
                m_scrollbar_alpha = 255;
                if (std::abs(m_scroll_y - max_scroll) < 0.5f) { m_scroll_y = max_scroll; m_velocity_y = 0; }
            } else if (std::abs(m_velocity_y) > MIN_VELOCITY) {
                m_velocity_y *= FRICTION;
                m_scrollbar_alpha = 255;
            } else {
                m_velocity_y = 0;
            }
        } else {
             if (m_scrollbar_alpha > 0) {
                m_scrollbar_alpha -= SCROLLBAR_FADE_SPEED;
                if (m_scrollbar_alpha < 0) m_scrollbar_alpha = 0;
            }
        }
    } else {
        m_scrollbar_alpha = 255;
    }

    Container::update();
}

void Layout::draw_content(Painter& painter) {
    painter.push_clip(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h);

    // Apply scroll offset via translation
    painter.push_translate(0, (int)m_scroll_y);
    
    // Virtual viewport in absolute coordinates shifted by scroll
    // Child Y is absolute
    // Visible Frame Top = m_bounds.y - m_scroll_y
    // Visible Frame Bottom = m_bounds.y - m_scroll_y + m_bounds.h
    // Add margin for focus outlines (e.g. 20px) to prevent culling artifacts
    int margin = 20;
    int visible_top = m_bounds.y - (int)m_scroll_y - margin;
    int visible_bottom = visible_top + m_bounds.h + (margin * 2);

    for (auto& child : m_children) {
        if (!child->visible()) continue;
        
        int child_y = child->bounds().y;
        int child_h = child->bounds().h;
        
        // Culling: Check intersection
        if (child_y + child_h >= visible_top && child_y <= visible_bottom) {
             child->draw(painter);
        }
    }
    painter.pop_translate();

    // Draw scrollbar (Inside Clip, Outside Translate)
    if (m_scrollbar_alpha > 0) {
        int total_content_height = content_height();
        if (total_content_height > m_bounds.h) {
            float view_ratio = (float)m_bounds.h / total_content_height;
            int max_scroll = -(total_content_height - m_bounds.h);
            
            float bar_h = (float)m_bounds.h * view_ratio;
            float bar_y;

            if (m_scroll_y > 0) {
                float overscroll = m_scroll_y;
                float size_factor = 1.0f - (overscroll / (float)m_bounds.h);
                if (size_factor < 0.2f) size_factor = 0.2f;
                bar_h *= size_factor;
                bar_y = (float)m_bounds.y;
            } else if (m_scroll_y < max_scroll) {
                float overscroll = max_scroll - m_scroll_y;
                float size_factor = 1.0f - (overscroll / (float)m_bounds.h);
                if (size_factor < 0.2f) size_factor = 0.2f;
                bar_h *= size_factor;
                bar_y = (float)m_bounds.y + (float)m_bounds.h - bar_h;
            } else {
                bar_y = (float)m_bounds.y + (-m_scroll_y * view_ratio);
            }

            Color thumb(150, 150, 150, (uint8_t)m_scrollbar_alpha);
            painter.fill_rect(m_bounds.x + m_bounds.w - 6, (int)bar_y, 4, (int)bar_h, thumb);
        }
    }

    painter.pop_clip();
}

void Layout::draw_focus(Painter& painter) {
    painter.push_clip(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h);
    painter.push_translate(0, (int)m_scroll_y);
    
    for (auto& child : m_children) {
        if (child->visible())
            child->draw_focus(painter);
    }
    
    painter.pop_translate();
    painter.pop_clip();
    
    Widget::draw_focus(painter);
}

bool Layout::on_touch(int tx, int ty, bool down, bool captured) {
    if (!m_visible) return false;

    // Adjust touch coordinate for scroll offset when checking children
    int adjusted_ty = ty - (int)m_scroll_y;

    // On initial press, setup state
    if (down && !m_prev_touch_down) {
        m_initial_touch_x = tx;
        m_initial_touch_y = ty;
        m_potential_swipe = m_bounds.contains(tx, ty);
        m_has_intercepted = false;
        m_is_dragging = false;
        m_last_touch_y = ty;
    }

    // If we've intercepted the touch for scrolling, handle it directly
    if (m_has_intercepted) {
        bool res = on_touch_event(tx - m_bounds.x, ty - m_bounds.y, down);
        if (!down) {
            m_has_intercepted = false;
            m_potential_swipe = false;
        }
        m_prev_touch_down = down;
        return res;
    }

    // Let container dispatch to children first
    bool result = Container::on_touch(tx, adjusted_ty, down, captured);

    // Check for swipe interception
    if (down && m_potential_swipe && !m_has_intercepted) {
        // Don't intercept if a scrollable child is handling it
        if (m_captured_child && m_captured_child->is_scrollable()) {
            m_potential_swipe = false;
        } else {
            int dx = std::abs(tx - m_initial_touch_x);
            int dy = std::abs(ty - m_initial_touch_y);
            const int SLOP = 10;

            if (dy > SLOP && dy > dx) {
                // Intercept for scrolling
                m_has_intercepted = true;
                if (m_captured_child) {
                    m_captured_child->cancel_gesture();
                    m_captured_child->on_touch(-10000, -10000, false, true);
                    m_captured_child = nullptr;
                }
                m_is_dragging = true;
                result = true;
            } else if (dx > SLOP) {
                m_potential_swipe = false;
            }
        }
    }

    // If no child handled and we're pressing on the layout, start dragging
    if (!result && down && m_bounds.contains(tx, ty)) {
        result = on_touch_event(tx - m_bounds.x, ty - m_bounds.y, down);
    }

    // Handle release
    if (!down) {
        if (m_is_dragging) {
            on_touch_event(tx - m_bounds.x, ty - m_bounds.y, down);
        }
        m_potential_swipe = false;
    }

    m_prev_touch_down = down;
    return result || m_has_intercepted;
}

bool Layout::on_touch_event(int local_x, int local_y, bool down) {
    int ty = m_bounds.y + local_y; 
    
    int total_content_height = content_height();
    int max_scroll = (total_content_height > m_bounds.h) ? -(total_content_height - m_bounds.h) : 0;
    
    if (down) {
        m_scrollbar_alpha = 255;
        if (!m_is_dragging) {
            m_is_dragging = true;
            m_velocity_y = 0;
        } else {
            float diff = (float)(ty - m_last_touch_y);
            if (m_scroll_y > 0 || m_scroll_y < max_scroll) {
                diff *= 0.5f; 
            }
            m_velocity_y = diff;
            m_scroll_y += m_velocity_y;
        }
        m_last_touch_y = ty;
        return true; 
    } else {
        if (m_is_dragging) {
            m_is_dragging = false;
        }
        return false;
    }
}

} // namespace Izo