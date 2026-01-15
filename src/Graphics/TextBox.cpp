// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "TextBox.hpp"
#include "../Core/Theme.hpp"
#include "../Input/Input.hpp"
#include <algorithm>
#include <cmath>

namespace Izo {

TextBox::TextBox(const std::string& placeholder, Font* font) 
    : m_text_buffer(""), m_placeholder(placeholder), m_font(font) {}

void TextBox::set_text(const std::string& t) { 
    if (m_text_buffer != t) {
        m_text_buffer = t;
        m_sel_start = m_sel_end = (int)t.length();
        ensure_cursor_visible();
        Widget::invalidate();
    }
}

const std::string& TextBox::text() const { return m_text_buffer; }

int TextBox::get_cursor_index(int lx) {
    if (!m_font || m_text_buffer.empty()) return 0;
    int best_idx = 0;
    int best_dist = 10000;
    int cur_x = 0;
    
    for (size_t i = 0; i <= m_text_buffer.length(); ++i) {
        int dist = abs(cur_x - lx);
        if (dist < best_dist) {
            best_dist = dist;
            best_idx = (int)i;
        }
        if (i < m_text_buffer.length()) {
            std::string s = m_text_buffer.substr(i, 1);
            cur_x += m_font->width(s);
        }
    }
    return best_idx;
}

void TextBox::ensure_cursor_visible() {
    if (!m_font) return;
    
    std::string pre_cursor = m_text_buffer.substr(0, m_sel_end);
    int cursor_x = m_font->width(pre_cursor);
    int visible_w = m_bounds.w - 10;
    
    if (cursor_x < m_scroll_x) {
        m_scroll_x = cursor_x;
    }
    else if (cursor_x > m_scroll_x + visible_w) {
        m_scroll_x = cursor_x - visible_w;
    }
    
    int total_w = m_font->width(m_text_buffer);
    if (total_w < visible_w) m_scroll_x = 0;
    else if (m_scroll_x > total_w - visible_w) m_scroll_x = total_w - visible_w;
    if (m_scroll_x < 0) m_scroll_x = 0;
}

void TextBox::draw_content(Painter& painter) {
    painter.fill_rect(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h, Theme::instance().color("TextBox.Background"));

    Color border = m_border_anim.value();
    painter.draw_rect(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h, border);

    if (m_font) {
        int padding = 5;
        painter.set_clip(m_bounds.x + padding, m_bounds.y + padding, m_bounds.w - 2 * padding, m_bounds.h - 2 * padding);
        
        int draw_x = m_bounds.x + padding - m_scroll_x;
        int draw_y = m_bounds.y + padding;
        
        if (m_sel_start != m_sel_end) {
            int s = std::min(m_sel_start, m_sel_end);
            int e = std::max(m_sel_start, m_sel_end);
            
            std::string pre_sel = m_text_buffer.substr(0, s);
            std::string sel_txt = m_text_buffer.substr(s, e - s);
            
            int x1 = m_font->width(pre_sel);
            int sw = m_font->width(sel_txt);
            
            painter.fill_rect(draw_x + x1, draw_y, sw, m_font->height(), Theme::instance().color("TextBox.Selection"));
        }

        if (m_text_buffer.empty()) {
            m_font->draw_text(painter, draw_x, draw_y, m_placeholder, Theme::instance().color("TextBox.Placeholder"));
        } else {
            m_font->draw_text(painter, draw_x, draw_y, m_text_buffer, Theme::instance().color("TextBox.Text"));
        }
        
        if (m_is_focused) {
             std::string pre_cursor = m_text_buffer.substr(0, m_sel_end);
             int cx = m_font->width(pre_cursor);
             painter.fill_rect(draw_x + cx, draw_y, 2, m_font->height(), Theme::instance().color("TextBox.Cursor"));
        }
        
        painter.reset_clip();
    }
}

void TextBox::update() {
    if (m_border_anim.update(16.0f)) Widget::invalidate();
    Widget::update(); 
}

bool TextBox::on_touch_event(int local_x, int local_y, bool down) {
    bool inside = (local_x >= 0 && local_x < m_bounds.w && local_y >= 0 && local_y < m_bounds.h);
    
    if (down) {
        if (inside) {
            int text_local_x = local_x - 5 + m_scroll_x;
            int idx = get_cursor_index(text_local_x);
            if (!m_is_dragging) {
                m_sel_start = idx;
                m_sel_end = idx;
                m_is_dragging = true;
            } else {
                m_sel_end = idx;
            }
            ensure_cursor_visible();
            Widget::invalidate();
            return true;
        } else {
             m_is_dragging = false;
        }
    } else {
        m_is_dragging = false;
    }
    return false;
}

bool TextBox::on_key(int key) {
    if (!m_is_focused) return false;
    // ... same key logic ...
    bool changed = false;
    bool shift = Input::instance().shift();

    if (key == Input::Backspace) { 
        if (m_sel_start != m_sel_end) {
            int s = std::min(m_sel_start, m_sel_end);
            int e = std::max(m_sel_start, m_sel_end);
            m_text_buffer.erase(s, e - s);
            m_sel_start = m_sel_end = s;
            changed = true;
        } else {
            if (m_sel_end > 0) {
                m_text_buffer.erase(m_sel_end - 1, 1);
                m_sel_end--;
                m_sel_start = m_sel_end;
                changed = true;
            }
        }
    } else if (key == Input::Left) {
        if (m_sel_end > 0) {
            m_sel_end--;
            if (!shift) m_sel_start = m_sel_end; 
            ensure_cursor_visible();
            Widget::invalidate();
        }
    } else if (key == Input::Right) {
        if (m_sel_end < (int)m_text_buffer.length()) {
            m_sel_end++;
            if (!shift) m_sel_start = m_sel_end;
            ensure_cursor_visible();
            Widget::invalidate();
        }
    } else if (key >= 32 && key < 127) {
        if (m_sel_start != m_sel_end) {
            int s = std::min(m_sel_start, m_sel_end);
            int e = std::max(m_sel_start, m_sel_end);
            m_text_buffer.erase(s, e - s);
            m_sel_start = m_sel_end = s;
        }
        m_text_buffer.insert(m_sel_end, 1, (char)key);
        m_sel_end++;
        m_sel_start = m_sel_end;
        changed = true;
    }
    
    if (changed) {
        ensure_cursor_visible();
        Widget::invalidate();
    }
    return true;
}

void TextBox::measure(int parent_w, int parent_h) {
    int mh = m_font ? m_font->height() + 10 : 30;
    m_measured_size = {0, 0, 200, mh};
}

} // namespace Izo