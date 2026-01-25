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

    // Removed direct input polling for scroll here.
    // Handling is moved to on_scroll.

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

bool Layout::on_scroll(int y) {
    if (!m_visible) return false;
    
    // First, let children handle it if they are hovered
    if (Container::on_scroll(y)) return true;
    
    // Check if we are hovered and have content overflow or are actively scrolling
    int total_content_height = content_height();
    if (total_content_height <= m_bounds.h) return false;
    
    IntPoint mouse = Input::the().touch_point();
    if (bounds().contains(mouse)) {
        if (y != 0) {
            m_velocity_y += (float)y * 12.0f;
            m_scrollbar_alpha = 255;
            return true;
        }
    }
    
    return false;
}

void Layout::draw_content(Painter& painter) {
    IntRect b = bounds();
    painter.push_clip(b);

    // Virtual viewport in absolute coordinates shifted by scroll
    // Child Y is absolute (screen coords now, via bounds())
    // Visible Frame Top = b.y
    // Visible Frame Bottom = b.y + b.h
    // Add margin for focus outlines (e.g. 20px) to prevent culling artifacts
    int margin = 20;
    int visible_top = b.y - margin;
    int visible_bottom = b.y + b.h + margin;

    for (auto& child : m_children) {
        if (!child->visible()) continue;
        
        IntRect cb = child->bounds();
        int child_y = cb.y;
        int child_h = cb.h;
        
        // Culling: Check intersection
        if (child_y + child_h >= visible_top && child_y <= visible_bottom) {
             child->draw(painter);
        }
    }
    
    // Draw scrollbar (Inside Clip)
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
                bar_y = (float)b.y;
            } else if (m_scroll_y < max_scroll) {
                float overscroll = max_scroll - m_scroll_y;
                float size_factor = 1.0f - (overscroll / (float)m_bounds.h);
                if (size_factor < 0.2f) size_factor = 0.2f;
                bar_h *= size_factor;
                bar_y = (float)b.y + (float)m_bounds.h - bar_h;
            } else {
                bar_y = (float)b.y + (-m_scroll_y * view_ratio);
            }

            Color thumb(150, 150, 150, (uint8_t)m_scrollbar_alpha);
            painter.fill_rect({b.x + b.w - 6, (int)bar_y, 4, (int)bar_h}, thumb);
        }
    }

    painter.pop_clip();
}

void Layout::draw_focus(Painter& painter) {
    painter.push_clip(bounds());
    // painter.push_translate({0, (int)m_scroll_y}); // Removed translation
    
    for (auto& child : m_children) {
        if (child->visible())
            child->draw_focus(painter);
    }
    
    // painter.pop_translate();
    painter.pop_clip();
    
    Widget::draw_focus(painter);
}

bool Layout::on_touch(IntPoint point, bool down, bool captured) {
    if (!m_visible) return false;

    // Adjust touch coordinate for scroll offset when checking children
    // IntPoint adjusted_point = { point.x, point.y - (int)m_scroll_y }; // REMOVED: bounds() handles adjusted coords
    IntPoint adjusted_point = point;

    // On initial press, setup state
    if (down && !m_prev_touch_down) {
        m_initial_touch_x = point.x;
        m_initial_touch_y = point.y;
        m_potential_swipe = bounds().contains(point);
        m_has_intercepted = false;
        m_is_dragging = false;
        m_last_touch_y = point.y;
    }

    // If we've intercepted the touch for scrolling, handle it directly
    if (m_has_intercepted) {
        bool res = on_touch_event({point.x - bounds().x, point.y - bounds().y}, down);
        if (!down) {
            m_has_intercepted = false;
            m_potential_swipe = false;
        }
        m_prev_touch_down = down;
        return res;
    }

    // Let container dispatch to children first
    bool result = Container::on_touch(adjusted_point, down, captured);

    // Check for swipe interception
    if (down && m_potential_swipe && !m_has_intercepted) {
        // Don't intercept if a scrollable child is handling it
        if (m_captured_child && m_captured_child->is_scrollable()) {
            m_potential_swipe = false;
        } else {
            int dx = std::abs(point.x - m_initial_touch_x);
            int dy = std::abs(point.y - m_initial_touch_y);
            const int SLOP = 10;

            if (dy > SLOP && dy > dx) {
                // Intercept for scrolling
                m_has_intercepted = true;
                if (m_captured_child) {
                    m_captured_child->cancel_gesture();
                    m_captured_child->on_touch({-10000, -10000}, false, true);
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
    if (!result && down && bounds().contains(point)) {
        result = on_touch_event({point.x - bounds().x, point.y - bounds().y}, down);
    }

    // Handle release
    if (!down) {
        if (m_is_dragging) {
            on_touch_event({point.x - bounds().x, point.y - bounds().y}, down);
        }
        m_potential_swipe = false;
    }

    m_prev_touch_down = down;
    return result || m_has_intercepted;
}

bool Layout::on_touch_event(IntPoint point, bool down) {
    // point is relative to bounds() TopLeft.
    // m_bounds.y is usually static layout pos. 
    // We want delta from visual touch.
    // If point is (0,0), that means touched at Visual Top Left.
    // ty should represent global Y or consistent local Y?
    // m_last_touch_y was stored as global Y in on_touch? No:
    // m_last_touch_y = point.y (Global) in on_touch line 151.
    // Here point is local.
    // Let's rely on Screen Coords for scrolling math to avoid confusion.
    // But on_touch passes local point.
    
    // Actually, on_touch lines:
    // m_last_touch_y = point.y (Global Point passed to on_touch).
    
    // on_touch_event is getting {point.x - bounds().x, ... }
    // So point is local.
    
    // To maintain existing logic:
    // ty = bounds().y + point.y;
    int ty = bounds().y + point.y; 
    
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