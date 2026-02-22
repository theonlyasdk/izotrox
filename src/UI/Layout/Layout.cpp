#include "Layout.hpp"
#include "Input/Input.hpp"
#include "Graphics/Painter.hpp"
#include "Graphics/Color.hpp"
#include "Core/Application.hpp"

#include <cmath>

namespace Izo {

constexpr float SCROLL_FRICTION = 0.96f;
constexpr float SCROLL_MIN_VELOCITY = 3.6f; // px/sec
constexpr float SCROLL_TENSION = 0.10f;
constexpr float SCROLLBAR_FADE_SPEED = 42.0f; // alpha/sec
constexpr float AUTO_SCROLL_SPEED = 7.5f;
constexpr float MOUSE_SCROLL_VELOCITY_SCALE = 450.0f; // px/sec per wheel step
constexpr int   TOUCH_SLOP_PX = 12;
constexpr int   SCROLLBAR_WIDTH_PX = 4;
constexpr int   SCROLLBAR_INSET_PX = 6;
constexpr int   VISIBILITY_MARGIN_PX = 20;
constexpr float OVERSCROLL_DAMPING = 0.30f;
constexpr float MIN_OVERSCROLL_BAR_FACTOR = 0.3f;
constexpr float AUTO_SCROLL_SNAP_EPSILON = 0.75f;
constexpr float AUTO_SCROLL_VELOCITY_EPS = 0.75f;

void Layout::measure(int parent_w, int parent_h) { 
    m_measured_size = {0, 0, 0, 0};
    if (m_width == (int)WidgetSizePolicy::MatchParent) m_measured_size.w = parent_w;
    if (m_height == (int)WidgetSizePolicy::MatchParent) m_measured_size.h = parent_h;
}

void Layout::smooth_scroll_to(int target_y) {
    m_auto_scrolling = true;
    m_auto_scroll_target = (float)target_y;

    int total_content_height = content_height();
    int max_scroll = (total_content_height > global_bounds().h) ? -(total_content_height - global_bounds().h) : 0;

    if (m_auto_scroll_target > 0.0f) m_auto_scroll_target = 0.0f;
    if (m_auto_scroll_target < (float)max_scroll) m_auto_scroll_target = (float)max_scroll;

    m_velocity_y = 0.0f;
    m_scrollbar_alpha = 255;
}

void Layout::update() {
    const float prev_scroll_y = m_scroll_y;
    const float prev_velocity_y = m_velocity_y;
    const float prev_scrollbar_alpha = m_scrollbar_alpha;
    const bool prev_auto_scrolling = m_auto_scrolling;
    const bool prev_is_dragging = m_is_dragging;

    int total_content_height = content_height();
    int max_scroll = (total_content_height > global_bounds().h) ? -(total_content_height - global_bounds().h) : 0;
    float dt_sec = Application::the().delta() * 0.001f;
    dt_sec = std::clamp(dt_sec, 0.0f, 0.1f);
    const float dt60 = dt_sec * 60.0f;

    if (m_auto_scrolling) {
        float diff = m_auto_scroll_target - m_scroll_y;
        m_velocity_y = diff * AUTO_SCROLL_SPEED;

        if (std::abs(diff) < AUTO_SCROLL_SNAP_EPSILON && std::abs(m_velocity_y) < AUTO_SCROLL_VELOCITY_EPS) {
            m_scroll_y = m_auto_scroll_target;
            m_velocity_y = 0;
            m_auto_scrolling = false;
        }
    }

    if (!m_is_dragging) {
        if (std::abs(m_velocity_y) > 0.001f || m_scroll_y > 0 || m_scroll_y < max_scroll || m_auto_scrolling) {
            m_scroll_y += m_velocity_y * dt_sec;

            if (!m_auto_scrolling) {
                if (m_scroll_y > 0) {
                    m_velocity_y = (0 - m_scroll_y) * SCROLL_TENSION * 60.0f;
                    m_scrollbar_alpha = 255;
                    if (std::abs(m_scroll_y) < 0.5f) { m_scroll_y = 0; m_velocity_y = 0; }
                } else if (m_scroll_y < max_scroll) {
                    m_velocity_y = (max_scroll - m_scroll_y) * SCROLL_TENSION * 60.0f;
                    m_scrollbar_alpha = 255;
                    if (std::abs(m_scroll_y - max_scroll) < 0.5f) { m_scroll_y = max_scroll; m_velocity_y = 0; }
                } else if (std::abs(m_velocity_y) > SCROLL_MIN_VELOCITY) {
                    m_velocity_y *= std::pow(SCROLL_FRICTION, dt60);
                    m_scrollbar_alpha = 255;
                } else {
                    m_velocity_y = 0;
                }
            } else {
                 m_scrollbar_alpha = 255;
                 if (m_scroll_y > 0.0f) m_scroll_y = 0.0f;
                 if (m_scroll_y < (float)max_scroll) m_scroll_y = (float)max_scroll;
            }
        } else {
             if (m_scrollbar_alpha > 0) {
                m_scrollbar_alpha -= SCROLLBAR_FADE_SPEED * dt_sec;
                if (m_scrollbar_alpha < 0) m_scrollbar_alpha = 0;
            }
        }
    } else {
        m_auto_scrolling = false; 
        m_scrollbar_alpha = 255;
    }

    Container::update();

    if (std::abs(m_scroll_y - prev_scroll_y) > 0.01f ||
        std::abs(m_velocity_y - prev_velocity_y) > 0.01f ||
        std::abs(m_scrollbar_alpha - prev_scrollbar_alpha) > 0.01f ||
        m_auto_scrolling != prev_auto_scrolling ||
        m_is_dragging != prev_is_dragging) {
        invalidate_visual();
    }
}

bool Layout::on_scroll(int y) {
    if (!m_visible) return false;

    if (Container::on_scroll(y)) return true;

    int total_content_height = content_height();
    if (total_content_height <= local_bounds().h) return false;

    IntPoint mouse = Input::the().touch_point();
    if (global_bounds().contains(mouse)) {
        if (y != 0) {
            m_velocity_y += (float)y * MOUSE_SCROLL_VELOCITY_SCALE;
            m_scrollbar_alpha = 255;
            invalidate_visual();
            return true;
        }
    }

    return false;
}

void Layout::draw_content(Painter& painter) {
    IntRect b = global_bounds();
    painter.push_clip(b);

    int margin = VISIBILITY_MARGIN_PX;
    int visible_top = b.y - margin;
    int visible_bottom = b.y + b.h + margin;

    for (auto& child : m_children) {
        if (!child->visible()) continue;

        IntRect cb = child->global_bounds();
        int child_y = cb.y;
        int child_h = cb.h;

        if (child_y + child_h >= visible_top && child_y <= visible_bottom) {
             child->draw(painter);
        }
    }

    if (m_scrollbar_alpha > 0) {
        int total_content_height = content_height();
        if (total_content_height > local_bounds().h) {
            float view_ratio = (float)local_bounds().h / total_content_height;
            int max_scroll = -(total_content_height - local_bounds().h);

            float bar_h = (float)local_bounds().h * view_ratio;
            float bar_y;

            if (m_scroll_y > 0) {
                float overscroll = m_scroll_y;
                float size_factor = 1.0f - (overscroll / (float)local_bounds().h);
                if (size_factor < MIN_OVERSCROLL_BAR_FACTOR) size_factor = MIN_OVERSCROLL_BAR_FACTOR;
                bar_h *= size_factor;
                bar_y = (float)b.y;
            } else if (m_scroll_y < max_scroll) {
                float overscroll = max_scroll - m_scroll_y;
                float size_factor = 1.0f - (overscroll / (float)local_bounds().h);
                if (size_factor < MIN_OVERSCROLL_BAR_FACTOR) size_factor = MIN_OVERSCROLL_BAR_FACTOR;
                bar_h *= size_factor;
                bar_y = (float)b.y + (float)local_bounds().h - bar_h;
            } else {
                bar_y = (float)b.y + (-m_scroll_y * view_ratio);
            }

            Color thumb(150, 150, 150, (uint8_t)m_scrollbar_alpha);
            painter.fill_rect({b.x + b.w - SCROLLBAR_INSET_PX, (int)bar_y, SCROLLBAR_WIDTH_PX, (int)bar_h}, thumb);
        }
    }

    painter.pop_clip();
}

void Layout::draw_focus(Painter& painter) {
    painter.push_clip(global_bounds());

    for (auto& child : m_children) {
        if (child->visible())
            child->draw_focus(painter);
    }

    painter.pop_clip();

    Widget::draw_focus(painter);
}

bool Layout::on_touch(IntPoint point, bool down, bool captured) {
    if (!m_visible) return false;

    IntPoint adjusted_point = point;

    if (down && !m_prev_touch_down) {
        m_initial_touch_x = point.x;
        m_initial_touch_y = point.y;
        m_potential_swipe = global_bounds().contains(point);
        m_has_intercepted = false;
        m_is_dragging = false;
        m_last_touch_y = point.y;
    }

    if (m_has_intercepted) {
        bool res = on_touch_event({point.x - global_bounds().x, point.y - global_bounds().y}, down);
        if (!down) {
            m_has_intercepted = false;
            m_potential_swipe = false;
        }
        m_prev_touch_down = down;
        return res;
    }

    bool result = Container::on_touch(adjusted_point, down, captured);

    if (down && m_potential_swipe && !m_has_intercepted) {

        if (m_captured_child && m_captured_child->is_scrollable()) {
            m_potential_swipe = false;
        } else {
            int dx = std::abs(point.x - m_initial_touch_x);
            int dy = std::abs(point.y - m_initial_touch_y);

            if (dy > TOUCH_SLOP_PX && dy > dx) {
                m_has_intercepted = true;
                if (m_captured_child) {
                    m_captured_child->cancel_gesture();
                    m_captured_child->on_touch({-10000, -10000}, false, true);
                    m_captured_child = nullptr;
                }
                m_is_dragging = true;
                result = true;
            } else if (dx > TOUCH_SLOP_PX) {
                m_potential_swipe = false;
            }
        }
    }

    if (!result && down && global_bounds().contains(point)) {
        result = on_touch_event({point.x - global_bounds().x, point.y - global_bounds().y}, down);
    }

    if (!down) {
        if (m_is_dragging) {
            on_touch_event({point.x - global_bounds().x, point.y - global_bounds().y}, down);
        }
        m_potential_swipe = false;
    }

    handle_focus_logic(global_bounds().contains(point), down);

    m_prev_touch_down = down;
    return result || m_has_intercepted;
}

bool Layout::on_touch_event(IntPoint point, bool down) {

    int ty = global_bounds().y + point.y; 

    int total_content_height = content_height();
    int max_scroll = (total_content_height > local_bounds().h) ? -(total_content_height - local_bounds().h) : 0;

    if (down) {
        m_scrollbar_alpha = 255;
        if (!m_is_dragging) {
            m_is_dragging = true;
            m_velocity_y = 0;
        } else {
            float diff = (float)(ty - m_last_touch_y);
            if (m_scroll_y > 0 || m_scroll_y < max_scroll) {
                diff *= OVERSCROLL_DAMPING; 
            }
            float dt_sec = Application::the().delta() * 0.001f;
            dt_sec = std::clamp(dt_sec, 0.001f, 0.1f);
            m_velocity_y = diff / dt_sec;
            m_scroll_y += diff;
        }
        m_last_touch_y = ty;
        invalidate_visual();
        return true; 
    } else {
        if (m_is_dragging) {
            m_is_dragging = false;
            invalidate_visual();
        }
        return false;
    }
}

bool Layout::has_running_animations() const {
    if (Container::has_running_animations()) return true;

    return m_is_dragging || m_auto_scrolling ||
           std::abs(m_velocity_y) > 0.01f ||
           std::abs(m_scrollbar_alpha) > 0.01f;
}

}
