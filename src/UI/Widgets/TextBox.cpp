#include "UI/Widgets/TextBox.hpp"
#include "Graphics/Font.hpp"
#include "Core/ThemeDB.hpp"
#include "Input/Input.hpp"
#include "Core/Application.hpp"
#include <algorithm>
#include <cmath>
#include <cctype>

namespace Izo {

std::string TextBox::s_clipboard;

TextBox::TextBox(const std::string& placeholder)
    : m_text_buffer(""), m_placeholder(placeholder) {}

void TextBox::set_text(const std::string& t) {
    if (m_text_buffer != t) {
        m_text_buffer = t;
        m_sel_start = m_sel_end = (int)t.length();
        ensure_cursor_visible();
        if (m_on_change) {
            m_on_change(m_text_buffer);
        }
    }
}

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

int TextBox::find_word_start(int pos) {
    if (pos <= 0) return 0;
    int i = pos - 1;
    while (i >= 0 && std::isspace(m_text_buffer[i])) i--;
    while (i >= 0 && !std::isspace(m_text_buffer[i])) i--;
    return i + 1;
}

int TextBox::find_word_end(int pos) {
    int len = (int)m_text_buffer.length();
    if (pos >= len) return len;
    int i = pos;
    while (i < len && std::isspace(m_text_buffer[i])) i++;
    while (i < len && !std::isspace(m_text_buffer[i])) i++;
    return i;
}

void TextBox::ensure_cursor_visible() {
    if (!m_font) return;

    std::string pre_cursor = m_text_buffer.substr(0, m_sel_end);
    int cursor_x = m_font->width(pre_cursor);
    int visible_w = m_bounds.w - 12;

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

    m_cursor_timer = 0.0f;
    m_cursor_visible = true;
}

void TextBox::draw_content(Painter& painter) {
    IntRect bounds = global_bounds();

    int roundness = ThemeDB::the().get<int>("WidgetParams", "Widget.Roundness", 6);

    Color color_bg = ThemeDB::the().get<Color>("Colors", "TextBox.Background", Color(100));
    Color color_selection = ThemeDB::the().get<Color>("Colors", "TextBox.Selection", Color(100));
    Color color_placeholder = ThemeDB::the().get<Color>("Colors", "TextBox.Placeholder", Color(100));
    Color color_text = ThemeDB::the().get<Color>("Colors", "TextBox.Text", Color(0));
    Color color_cursor = ThemeDB::the().get<Color>("Colors", "TextBox.Cursor", Color(255));
    Color color_border = m_border_anim.value();

    painter.fill_rounded_rect(bounds, roundness, color_bg);
    painter.draw_rounded_rect(bounds, roundness, color_border);

    if (m_font) {
        int padding = 5;
        IntRect clip = {bounds.x + padding, bounds.y + padding, bounds.w - 2 * padding, bounds.h - 2 * padding};
        painter.push_clip(clip);

        int draw_x = bounds.x + padding - m_scroll_x;
        int draw_y = bounds.y + padding;

        if (m_text_buffer.empty()) {
            m_font->draw_text(painter, {draw_x, draw_y}, m_placeholder, color_placeholder);
        } else {
            int visible_w = bounds.w - 2 * padding;
            int start_idx = 0;
            int end_idx = (int)m_text_buffer.length();

            int cur_w = 0;
            for (int i = 0; i < (int)m_text_buffer.length(); ++i) {
                int char_w = m_font->width(m_text_buffer.substr(i, 1));
                if (cur_w + char_w < m_scroll_x) {
                    start_idx = i + 1;
                }
                cur_w += char_w;
                if (cur_w > m_scroll_x + visible_w) {
                    end_idx = i + 1;
                    break;
                }
            }

            if (m_sel_start != m_sel_end) {
                int s = std::min(m_sel_start, m_sel_end);
                int e = std::max(m_sel_start, m_sel_end);

                int x1 = m_font->width(m_text_buffer.substr(0, s));
                int sw = m_font->width(m_text_buffer.substr(s, e - s));

                painter.fill_rect({draw_x + x1, draw_y, sw, m_font->height()}, color_selection);
            }

            std::string visible_text = m_text_buffer.substr(start_idx, end_idx - start_idx);
            int offset_x = m_font->width(m_text_buffer.substr(0, start_idx));
            m_font->draw_text(painter, {draw_x + offset_x, draw_y}, visible_text, color_text);
        }

        if (m_focused && m_cursor_visible) {
             std::string pre_cursor = m_text_buffer.substr(0, m_sel_end);
             int cx = m_font->width(pre_cursor);
             painter.fill_rect({draw_x + cx, draw_y, 2, m_font->height()}, color_cursor);
        }

        painter.pop_clip();
    }
}

void TextBox::update() {
    m_border_anim.update(Application::the().delta());
    Widget::update();

    if (m_focused) {
        m_cursor_timer += Application::the().delta();
        int blink_speed = ThemeDB::the().get<int>("System", "CursorBlinkSpeed", 500);
        if (m_cursor_timer >= (float)blink_speed) {
            m_cursor_visible = !m_cursor_visible;
            m_cursor_timer = 0.0f;
        }

        if (m_is_dragging) {
            IntPoint mouse = Input::the().touch_point();
            IntRect b = global_bounds();
            int padding = 5;
            if (mouse.x < b.x + padding) {
                m_scroll_x -= 5;
                if (m_scroll_x < 0) m_scroll_x = 0;
                int text_local_x = mouse.x - b.x - padding + m_scroll_x;
                m_sel_end = get_cursor_index(text_local_x);
            } else if (mouse.x > b.x + b.w - padding) {
                int total_w = m_font->width(m_text_buffer);
                int visible_w = b.w - 2 * padding;
                m_scroll_x += 5;
                if (m_scroll_x > total_w - visible_w) m_scroll_x = std::max(0, total_w - visible_w);
                int text_local_x = mouse.x - b.x - padding + m_scroll_x;
                m_sel_end = get_cursor_index(text_local_x);
            }
        }
    } else {
        m_cursor_visible = false;
        m_cursor_timer = 0.0f;
    }
}

bool TextBox::on_touch_event(IntPoint point, bool down) {
    if (down) {
        int text_local_x = point.x - 5 + m_scroll_x;
        int idx = get_cursor_index(text_local_x);
        if (!m_is_dragging) {
            m_sel_start = idx;
            m_sel_end = idx;
            m_is_dragging = true;
        } else {
            m_sel_end = idx;
        }
        ensure_cursor_visible();
        return true;
    } else {
        m_is_dragging = false;
    }
    return false;
}

bool TextBox::on_key(KeyCode key) {
    if (!m_focused) return false;

    bool changed = false;
    bool shift = Input::the().shift();
    bool ctrl = Input::the().ctrl();
    int keyVal = (int)key;
    int len = (int)m_text_buffer.length();
    bool has_selection = (m_sel_start != m_sel_end);

    if (ctrl && keyVal == 'a') {
        m_sel_start = 0;
        m_sel_end = len;
        ensure_cursor_visible();
        return true;
    }

    if (ctrl && keyVal == 'c') {
        if (has_selection) {
            int s = std::min(m_sel_start, m_sel_end);
            int e = std::max(m_sel_start, m_sel_end);
            s_clipboard = m_text_buffer.substr(s, e - s);
        }
        return true;
    }

    if (ctrl && keyVal == 'x') {
        if (has_selection) {
            int s = std::min(m_sel_start, m_sel_end);
            int e = std::max(m_sel_start, m_sel_end);
            s_clipboard = m_text_buffer.substr(s, e - s);
            m_text_buffer.erase(s, e - s);
            m_sel_start = m_sel_end = s;
            changed = true;
        }
        ensure_cursor_visible();
        if (changed && m_on_change) m_on_change(m_text_buffer);
        return true;
    }

    if (ctrl && keyVal == 'v') {
        if (!s_clipboard.empty()) {
            if (has_selection) {
                int s = std::min(m_sel_start, m_sel_end);
                int e = std::max(m_sel_start, m_sel_end);
                m_text_buffer.erase(s, e - s);
                m_sel_start = m_sel_end = s;
            }
            m_text_buffer.insert(m_sel_end, s_clipboard);
            m_sel_end += (int)s_clipboard.length();
            m_sel_start = m_sel_end;
            changed = true;
        }
        ensure_cursor_visible();
        if (changed && m_on_change) m_on_change(m_text_buffer);
        return true;
    }

    if (key == KeyCode::Backspace) {
        if (has_selection) {
            int s = std::min(m_sel_start, m_sel_end);
            int e = std::max(m_sel_start, m_sel_end);
            m_text_buffer.erase(s, e - s);
            m_sel_start = m_sel_end = s;
            changed = true;
        } else if (m_sel_end > 0) {
            int target = ctrl ? find_word_start(m_sel_end) : (m_sel_end - 1);
            int count = m_sel_end - target;
            m_text_buffer.erase(target, count);
            m_sel_end = target;
            m_sel_start = m_sel_end;
            changed = true;
        }
    } else if (key == KeyCode::Delete) {
        if (has_selection) {
            int s = std::min(m_sel_start, m_sel_end);
            int e = std::max(m_sel_start, m_sel_end);
            m_text_buffer.erase(s, e - s);
            m_sel_start = m_sel_end = s;
            changed = true;
        } else if (m_sel_end < len) {
            int target = ctrl ? find_word_end(m_sel_end) : (m_sel_end + 1);
            int count = target - m_sel_end;
            m_text_buffer.erase(m_sel_end, count);
            changed = true;
        }
    } else if (key == KeyCode::Enter) {
        if (m_on_submit) {
            m_on_submit(m_text_buffer);
        }
        return true;
    } else if (key == KeyCode::Home) {
        m_sel_end = 0;
        if (!shift) m_sel_start = m_sel_end;
        ensure_cursor_visible();
    } else if (key == KeyCode::End) {
        m_sel_end = len;
        if (!shift) m_sel_start = m_sel_end;
        ensure_cursor_visible();
    } else if (key == KeyCode::Left) {
        if (has_selection && !shift) {
            m_sel_end = std::min(m_sel_start, m_sel_end);
            m_sel_start = m_sel_end;
        } else if (m_sel_end > 0) {
            m_sel_end = ctrl ? find_word_start(m_sel_end) : (m_sel_end - 1);
            if (!shift) m_sel_start = m_sel_end;
        }
        ensure_cursor_visible();
    } else if (key == KeyCode::Right) {
        if (has_selection && !shift) {
            m_sel_end = std::max(m_sel_start, m_sel_end);
            m_sel_start = m_sel_end;
        } else if (m_sel_end < len) {
            m_sel_end = ctrl ? find_word_end(m_sel_end) : (m_sel_end + 1);
            if (!shift) m_sel_start = m_sel_end;
        }
        ensure_cursor_visible();
    } else if (keyVal >= 32 && keyVal < 127 && !ctrl) {
        if (has_selection) {
            int s = std::min(m_sel_start, m_sel_end);
            int e = std::max(m_sel_start, m_sel_end);
            m_text_buffer.erase(s, e - s);
            m_sel_start = m_sel_end = s;
        }
        m_text_buffer.insert(m_sel_end, 1, (char)keyVal);
        m_sel_end++;
        m_sel_start = m_sel_end;
        changed = true;
    }

    if (changed) {
        ensure_cursor_visible();
        if (m_on_change) {
            m_on_change(m_text_buffer);
        }
    }
    return true;
}

void TextBox::measure(int parent_w, int parent_h) {
    int mh = m_font ? m_font->height() + 10 : 30;
    m_measured_size = {0, 0, 200, mh};
}



}
