// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "ListView.hpp"
#include "Core/ThemeDB.hpp"
#include "Input/Input.hpp"
#include <algorithm>
#include <cmath>
#include <linux/input-event-codes.h>
#include <linux/input.h>

namespace Izo {

ListView::ListView() {
    set_focusable(true);
}

void ListView::set_items(const std::vector<std::string>& items) {
    m_items = items;
    m_item_count = items.size();
    Widget::invalidate();
}

void ListView::set_item_drawer(std::function<void(Painter&, int index, int x, int y, int w, int h)> drawer) {
    m_drawer = drawer;
}

void ListView::set_item_count(int count) {
    m_item_count = count;
    Widget::invalidate();
}

void ListView::update() {
    int total_content_height = m_item_count * m_item_height;
    int max_scroll = (total_content_height > m_bounds.h) ? -(total_content_height - m_bounds.h) : 0;

    if (!m_is_dragging) {
        if (std::abs(m_velocity_y) > 0.001f || m_scroll_y > 0 || m_scroll_y < max_scroll) {
            m_scroll_y += m_velocity_y;
            Widget::invalidate();

            if (m_scroll_y > 0) {
                m_velocity_y = (0 - m_scroll_y) * TENSION;
                m_scrollbar_alpha = 255;
            } else if (m_scroll_y < max_scroll) {
                m_velocity_y = (max_scroll - m_scroll_y) * TENSION;
                m_scrollbar_alpha = 255;
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
                Widget::invalidate();
            }
        }
    } else {
        m_scrollbar_alpha = 255;
    }

    if(Input::the().key() == (KeyCode)'a') {
        m_selected_index--;
    }

    Widget::update();
}

void ListView::draw_content(Painter& painter) {
    Color bg = ThemeDB::the().color("ListView.Background");
    if (bg.a == 0) bg = ThemeDB::the().color("Window.Background");
    
    painter.fill_rect(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h, bg);

    painter.set_clip(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h);

    int start_idx = (int)(-m_scroll_y / m_item_height);
    if (start_idx < 0) start_idx = 0;
    
    int end_idx = start_idx + (m_bounds.h / m_item_height) + 2;
    if (end_idx > m_item_count) end_idx = m_item_count;

    Color divColor = ThemeDB::the().color("ListView.Divider");
    Color highlight = ThemeDB::the().color("ListView.Highlight");

    for (int i = start_idx; i < end_idx; ++i) {
        int item_y = m_bounds.y + (i * m_item_height) + (int)m_scroll_y;
        
        if (i == m_selected_index) {
             painter.fill_rect(m_bounds.x, item_y, m_bounds.w, m_item_height, highlight);
        }

        if (m_drawer) {
            m_drawer(painter, i, m_bounds.x, item_y, m_bounds.w, m_item_height);
        }
        
        painter.fill_rect(m_bounds.x, item_y + m_item_height - 1, m_bounds.w, 1, divColor);
    }
    
    painter.reset_clip();

    if (m_scrollbar_alpha > 0) {
        int total_content_height = m_item_count * m_item_height;
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

    if (!focused()) {
        painter.draw_rect(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h, ThemeDB::the().color("TextBox.Border"));
    }
}

bool ListView::on_touch_event(int local_x, int local_y, bool down) {
    int ty = m_bounds.y + local_y; 
    
    int total_content_height = m_item_count * m_item_height;
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
            Widget::invalidate();
        }
        m_last_touch_y = ty;
        return true; 
    } else {
        if (m_is_dragging) {
            m_is_dragging = false;
            if (std::abs(m_velocity_y) < 2.0f) {
                 int clicked_y = ty - m_bounds.y - (int)m_scroll_y;
                 int index = clicked_y / m_item_height;
                 if (index >= 0 && index < m_item_count) {
                     m_selected_index = index;
                     Widget::invalidate();
                 }
            }
            Widget::invalidate();
        }
        return false;
    }
}

void ListView::measure(int parent_w, int parent_h) {
    int w = parent_w;
    int h = parent_h;
    if (m_width == WrapContent) w = 200; 
    if (m_height == WrapContent) h = m_item_count * m_item_height; 
    
    m_measured_size = {0, 0, w, h};
}

} // namespace Izo
