#include "UI/Widgets/Label.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cmath>
#include <limits>

#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "Graphics/Font.hpp"
#include "Graphics/Painter.hpp"
#include "Input/Input.hpp"

namespace Izo {

constexpr float kMarqueeStartOffset = 5.0f;
constexpr float kMarqueeSpeedPxPerSec = 50.0f;
constexpr int kMultiClickIntervalMs = 350;
constexpr int kMultiClickSlopPx = 12;

static long long now_ms() {
    using clock = std::chrono::steady_clock;
    return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now().time_since_epoch()).count();
}

Label::Label(const std::string& text) : m_text(text) {
    set_widget_type("Label");
    on_theme_update();
}

void Label::set_text(const std::string& text) {
    if (m_text == text) return;
    m_text = text;
    m_should_scroll = false;
    m_scroll_anim.set_loop(false);
    m_scroll_anim.snap_to(0.0f);
    m_sel_start = std::clamp(m_sel_start, 0, static_cast<int>(m_text.size()));
    m_sel_end = std::clamp(m_sel_end, 0, static_cast<int>(m_text.size()));
    invalidate_layout();
}

void Label::set_wrap(bool wrap) {
    if (m_should_wrap == wrap) return;
    m_should_wrap = wrap;
    invalidate_layout();
}

void Label::set_alignment(TextAlign align) {
    if (m_alignment == align) return;
    m_alignment = align;
    invalidate_visual();
}

void Label::set_color_variant(ColorVariant variant) {
    if (m_color_variant == variant) return;
    m_color_variant = variant;
    invalidate_visual();
}

void Label::set_color(const Color& color) {
    if (m_has_custom_color && m_custom_color.as_argb() == color.as_argb()) {
        return;
    }
    m_has_custom_color = true;
    m_custom_color = color;
    invalidate_visual();
}

Color Label::color() const {
    if (m_has_custom_color) return m_custom_color;
    return ThemeDB::the().get_variant_color(m_color_variant);
}

void Label::on_theme_update() {
    Widget::on_theme_update();
    m_color_selection = ThemeDB::the().get<Color>("Colors", "TextBox.Selection", Color(100, 100, 100, 120));
    m_color_cursor = ThemeDB::the().get<Color>("Colors", "TextBox.Cursor", Color(255));
    m_cursor_blink_speed_ms = ThemeDB::the().get<int>("System", "CursorBlinkSpeed", 500);
}

void Label::build_layout_lines(std::vector<LineLayout>& out_lines, int& line_height) const {
    out_lines.clear();
    line_height = m_font ? std::max(1, m_font->height()) : 1;

    if (!m_font) {
        return;
    }

    int align_width = std::max(1, m_bounds.w);
    int wrap_width = m_should_wrap ? std::max(1, m_bounds.w) : -1;

    auto finalize_line = [&](const std::string& text, int start_idx, int end_idx, int y) {
        LineLayout line;
        line.text = text;
        line.start_idx = start_idx;
        line.end_idx = end_idx;
        line.width = m_font->width(text);
        line.y = y;

        int x = 0;
        if (align_width > 0) {
            if (m_alignment == TextAlign::Center) {
                x = (align_width - line.width) / 2;
            } else if (m_alignment == TextAlign::Right) {
                x = align_width - line.width;
            }
        }
        line.x = x;
        out_lines.push_back(std::move(line));
    };

    int cur_y = 0;

    auto process_source_line = [&](const std::string& source, int source_start_idx) {
        if (wrap_width <= 0) {
            finalize_line(source, source_start_idx, source_start_idx + static_cast<int>(source.size()), cur_y);
            cur_y += line_height;
            return;
        }

        std::string current_line;
        std::string word;
        int current_start_idx = source_start_idx;
        int word_start_idx = source_start_idx;
        int current_width = 0;

        auto flush_word = [&]() {
            if (word.empty()) return;

            int word_width = m_font->width(word);
            if (!current_line.empty() && current_width + word_width > wrap_width) {
                finalize_line(current_line, current_start_idx, current_start_idx + static_cast<int>(current_line.size()), cur_y);
                cur_y += line_height;

                current_line = word;
                current_width = word_width;
                current_start_idx = word_start_idx;
            } else {
                if (current_line.empty()) {
                    current_start_idx = word_start_idx;
                }
                current_line += word;
                current_width += word_width;
            }
            word.clear();
        };

        for (size_t i = 0; i < source.size(); ++i) {
            if (word.empty()) {
                word_start_idx = source_start_idx + static_cast<int>(i);
            }
            char c = source[i];
            word += c;
            if (c == ' ') {
                flush_word();
            }
        }
        flush_word();

        if (!current_line.empty() || source.empty()) {
            finalize_line(current_line, current_start_idx, current_start_idx + static_cast<int>(current_line.size()), cur_y);
            cur_y += line_height;
        }
    };

    size_t start = 0;
    size_t end = m_text.find('\n');
    while (end != std::string::npos) {
        process_source_line(m_text.substr(start, end - start), static_cast<int>(start));
        start = end + 1;
        end = m_text.find('\n', start);
    }
    process_source_line(m_text.substr(start), static_cast<int>(start));

    if (out_lines.empty()) {
        finalize_line("", 0, 0, 0);
    }
}

int Label::cursor_index_from_local_point(IntPoint local, const std::vector<LineLayout>& lines, int line_height) const {
    if (!m_font) return 0;
    if (lines.empty()) return 0;

    int line_idx = std::clamp(local.y / std::max(1, line_height), 0, static_cast<int>(lines.size()) - 1);
    const auto& line = lines[line_idx];

    if (line.text.empty()) {
        return line.start_idx;
    }

    int local_x = local.x - line.x;
    int best_idx = line.start_idx;
    int best_dist = std::numeric_limits<int>::max();

    int cur_x = 0;
    for (size_t i = 0; i <= line.text.size(); ++i) {
        int dist = std::abs(cur_x - local_x);
        if (dist < best_dist) {
            best_dist = dist;
            best_idx = line.start_idx + static_cast<int>(i);
        }

        if (i < line.text.size()) {
            cur_x += m_font->width(line.text.substr(i, 1));
        }
    }

    return std::clamp(best_idx, 0, static_cast<int>(m_text.size()));
}

IntPoint Label::cursor_local_position(int index, const std::vector<LineLayout>& lines) const {
    IntPoint pos{0, 0};
    if (!m_font || lines.empty()) return pos;

    index = std::clamp(index, 0, static_cast<int>(m_text.size()));

    for (size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        if (index >= line.start_idx && index <= line.end_idx) {
            int in_line = index - line.start_idx;
            in_line = std::clamp(in_line, 0, static_cast<int>(line.text.size()));
            std::string prefix = line.text.substr(0, static_cast<size_t>(in_line));
            pos.x = line.x + m_font->width(prefix);
            pos.y = line.y;
            return pos;
        }
    }

    const auto& last = lines.back();
    pos.x = last.x + m_font->width(last.text);
    pos.y = last.y;
    return pos;
}

void Label::reset_cursor_blink() {
    m_cursor_visible = true;
    m_cursor_timer = 0.0f;
}

int Label::word_start_at(int index) const {
    const int len = static_cast<int>(m_text.size());
    if (len == 0) return 0;

    index = std::clamp(index, 0, len);
    if (index == len) index = len - 1;

    auto is_space = [&](int i) -> bool {
        unsigned char ch = static_cast<unsigned char>(m_text[static_cast<size_t>(i)]);
        return std::isspace(ch) != 0;
    };

    bool target_is_space = is_space(index);
    int pos = index;
    while (pos > 0 && is_space(pos - 1) == target_is_space) {
        --pos;
    }
    return pos;
}

int Label::word_end_at(int index) const {
    const int len = static_cast<int>(m_text.size());
    if (len == 0) return 0;

    index = std::clamp(index, 0, len);
    if (index == len) index = len - 1;

    auto is_space = [&](int i) -> bool {
        unsigned char ch = static_cast<unsigned char>(m_text[static_cast<size_t>(i)]);
        return std::isspace(ch) != 0;
    };

    bool target_is_space = is_space(index);
    int pos = index;
    while (pos + 1 < len && is_space(pos + 1) == target_is_space) {
        ++pos;
    }
    return pos + 1;
}

int Label::paragraph_start_at(int index) const {
    const int len = static_cast<int>(m_text.size());
    if (len == 0) return 0;

    index = std::clamp(index, 0, len);
    if (index == len && index > 0) index--;
    if (index > 0 && index < len && m_text[static_cast<size_t>(index)] == '\n') index--;

    while (index > 0 && m_text[static_cast<size_t>(index - 1)] != '\n') {
        --index;
    }
    return index;
}

int Label::paragraph_end_at(int index) const {
    const int len = static_cast<int>(m_text.size());
    if (len == 0) return 0;

    index = std::clamp(index, 0, len);
    if (index == len && index > 0) index--;
    if (index > 0 && index < len && m_text[static_cast<size_t>(index)] == '\n') index--;

    while (index < len && m_text[static_cast<size_t>(index)] != '\n') {
        ++index;
    }
    return index;
}

void Label::apply_drag_selection(int current_index, int anchor_index, SelectionDragMode mode) {
    current_index = std::clamp(current_index, 0, static_cast<int>(m_text.size()));
    anchor_index = std::clamp(anchor_index, 0, static_cast<int>(m_text.size()));

    switch (mode) {
        case SelectionDragMode::Character:
            m_sel_start = anchor_index;
            m_sel_end = current_index;
            break;
        case SelectionDragMode::Word: {
            int anchor_s = word_start_at(anchor_index);
            int anchor_e = word_end_at(anchor_index);
            int current_s = word_start_at(current_index);
            int current_e = word_end_at(current_index);
            if (current_index < anchor_index) {
                m_sel_start = current_s;
                m_sel_end = anchor_e;
            } else {
                m_sel_start = anchor_s;
                m_sel_end = current_e;
            }
            break;
        }
        case SelectionDragMode::Paragraph: {
            int anchor_s = paragraph_start_at(anchor_index);
            int anchor_e = paragraph_end_at(anchor_index);
            int current_s = paragraph_start_at(current_index);
            int current_e = paragraph_end_at(current_index);
            if (current_index < anchor_index) {
                m_sel_start = current_s;
                m_sel_end = anchor_e;
            } else {
                m_sel_start = anchor_s;
                m_sel_end = current_e;
            }
            break;
        }
    }

    m_sel_start = std::clamp(m_sel_start, 0, static_cast<int>(m_text.size()));
    m_sel_end = std::clamp(m_sel_end, 0, static_cast<int>(m_text.size()));
}

void Label::update() {
    float dt = Application::the().delta();
    bool was_scrolling = m_should_scroll;
    float previous_offset = m_scroll_anim.value();
    bool old_cursor_visible = m_cursor_visible;
    int old_sel_start = m_sel_start;
    int old_sel_end = m_sel_end;
    bool old_selecting = m_selecting;

    if (m_selecting && m_drag_mode == SelectionDragMode::Character && !Input::the().shift()) {
        m_selecting = false;
    }

    if (!focused()) {
        m_selecting = false;
        if (m_sel_start != m_sel_end) {
            m_sel_start = m_sel_end;
        }
    }

    if (focused()) {
        m_cursor_timer += dt;
        if (m_cursor_timer >= static_cast<float>(m_cursor_blink_speed_ms)) {
            m_cursor_visible = !m_cursor_visible;
            m_cursor_timer = 0.0f;
        }
    } else {
        m_cursor_visible = false;
        m_cursor_timer = 0.0f;
    }

    bool selection_active = focused() || (m_sel_start != m_sel_end) || m_selecting;

    bool can_scroll = false;
    if (!selection_active && m_font && !m_should_wrap && m_text.find('\n') == std::string::npos && m_bounds.w > 0) {
        int text_width = m_font->width(m_text);
        can_scroll = text_width > m_bounds.w;

        if (can_scroll && !m_should_scroll) {
            m_should_scroll = true;
            float target = -static_cast<float>(text_width);
            float distance = kMarqueeStartOffset - target;
            float duration_ms = (distance / kMarqueeSpeedPxPerSec) * 1000.0f;

            m_scroll_anim = Animator<float>(kMarqueeStartOffset);
            m_scroll_anim.set_loop(true);
            m_scroll_anim.set_target(target, duration_ms, Easing::Linear);
        }
    }

    if (!can_scroll) {
        m_should_scroll = false;
    }

    if (!m_should_scroll) {
        m_scroll_anim.set_loop(false);
        m_scroll_anim.snap_to(0.0f);
    } else {
        m_scroll_anim.update(dt);
    }

    if (was_scrolling != m_should_scroll ||
        m_scroll_anim.running() ||
        previous_offset != m_scroll_anim.value() ||
        old_cursor_visible != m_cursor_visible ||
        old_sel_start != m_sel_start ||
        old_sel_end != m_sel_end ||
        old_selecting != m_selecting) {
        invalidate_visual();
    }
    Widget::update();
}

void Label::draw_content(Painter& painter) {
    if (!m_font) return;

    Color text_color = color();
    IntRect bounds = global_bounds();

    if (m_should_scroll) {
        painter.push_clip(bounds);
        int tx = bounds.x + static_cast<int>(m_scroll_anim.value());
        m_font->draw_text(painter, {tx, bounds.y}, m_text, text_color);
        painter.pop_clip();
        return;
    }

    std::vector<LineLayout> lines;
    int line_height = 1;
    build_layout_lines(lines, line_height);

    int s = std::min(m_sel_start, m_sel_end);
    int e = std::max(m_sel_start, m_sel_end);
    bool has_selection = (s != e);

    painter.push_clip(bounds);

    if (has_selection) {
        for (const auto& line : lines) {
            int ls = std::max(s, line.start_idx);
            int le = std::min(e, line.end_idx);
            if (ls >= le) continue;

            int start_off = ls - line.start_idx;
            int end_off = le - line.start_idx;

            std::string prefix = line.text.substr(0, static_cast<size_t>(start_off));
            std::string selected = line.text.substr(static_cast<size_t>(start_off), static_cast<size_t>(end_off - start_off));

            int x = bounds.x + line.x + m_font->width(prefix);
            int w = m_font->width(selected);
            painter.fill_rect({x, bounds.y + line.y, w, line_height}, m_color_selection);
        }
    }

    for (const auto& line : lines) {
        m_font->draw_text(painter, {bounds.x + line.x, bounds.y + line.y}, line.text, text_color);
    }

    if (focused() && m_cursor_visible) {
        IntPoint cursor_local = cursor_local_position(m_sel_end, lines);
        painter.fill_rect({bounds.x + cursor_local.x, bounds.y + cursor_local.y, 2, line_height}, m_color_cursor);
    }

    painter.pop_clip();
}

bool Label::on_touch_event(IntPoint point, bool down) {
    if (!m_font) return false;

    std::vector<LineLayout> lines;
    int line_height = 1;
    build_layout_lines(lines, line_height);

    if (down) {
        int current_index = cursor_index_from_local_point(point, lines, line_height);
        bool shift_down = Input::the().shift();

        if (!m_touch_down) {
            m_touch_down = true;
            m_vertical_nav_active = false;

            const long long click_ms = now_ms();
            int dx = point.x - m_last_click_point.x;
            int dy = point.y - m_last_click_point.y;
            bool within_click_window = (click_ms - m_last_click_ms) <= kMultiClickIntervalMs;
            bool within_click_slop = (dx * dx + dy * dy) <= (kMultiClickSlopPx * kMultiClickSlopPx);
            if (within_click_window && within_click_slop) {
                m_click_count = std::min(3, m_click_count + 1);
            } else {
                m_click_count = 1;
            }
            m_last_click_ms = click_ms;
            m_last_click_point = point;

            bool had_focus = focused();
            bool had_selection = (m_sel_start != m_sel_end);
            int old_sel_start = m_sel_start;
            int old_sel_end = m_sel_end;

            set_focused(true);
            reset_cursor_blink();

            if (m_click_count >= 3) {
                m_drag_mode = SelectionDragMode::Paragraph;
                m_drag_anchor = current_index;
                m_selecting = true;
                m_consume_touch = true;
                apply_drag_selection(current_index, m_drag_anchor, m_drag_mode);
                invalidate_visual();
                return true;
            }

            if (m_click_count == 2) {
                m_drag_mode = SelectionDragMode::Word;
                m_drag_anchor = current_index;
                m_selecting = true;
                m_consume_touch = true;
                apply_drag_selection(current_index, m_drag_anchor, m_drag_mode);
                invalidate_visual();
                return true;
            }

            if (shift_down) {
                m_drag_mode = SelectionDragMode::Character;
                if (m_sel_start != m_sel_end) {
                    m_drag_anchor = m_sel_start;
                } else {
                    m_drag_anchor = had_focus ? m_sel_end : current_index;
                }
                m_selecting = true;
                m_consume_touch = true;
                apply_drag_selection(current_index, m_drag_anchor, m_drag_mode);
                invalidate_visual();
                return true;
            }

            m_drag_mode = SelectionDragMode::Character;
            m_selecting = false;
            m_sel_start = current_index;
            m_sel_end = current_index;
            m_consume_touch = had_selection;
            if (had_selection || old_sel_start != m_sel_start || old_sel_end != m_sel_end || !had_focus) {
                invalidate_visual();
            }
            return m_consume_touch;
        }

        if (m_selecting) {
            int old_sel_start = m_sel_start;
            int old_sel_end = m_sel_end;
            apply_drag_selection(current_index, m_drag_anchor, m_drag_mode);
            if (old_sel_start != m_sel_start || old_sel_end != m_sel_end) {
                reset_cursor_blink();
                invalidate_visual();
            }
            return true;
        }

        return m_consume_touch;
    }

    if (!m_touch_down) return false;

    m_touch_down = false;
    bool consumed = m_consume_touch;
    m_consume_touch = false;

    if (m_selecting) {
        m_selecting = false;
        invalidate_visual();
        return true;
    }

    return consumed;
}

bool Label::on_key(KeyCode key) {
    if (!focused() || !m_font) return false;

    int old_sel_start = m_sel_start;
    int old_sel_end = m_sel_end;
    bool shift = Input::the().shift();
    bool ctrl = Input::the().ctrl();
    bool handled = false;

    int len = static_cast<int>(m_text.size());
    bool has_selection = (m_sel_start != m_sel_end);

    auto apply_cursor_move = [&](int new_index) {
        new_index = std::clamp(new_index, 0, len);
        m_sel_end = new_index;
        if (!shift) {
            m_sel_start = m_sel_end;
        }
        reset_cursor_blink();
        handled = true;
    };

    if (key == KeyCode::Home || key == KeyCode::End || key == KeyCode::Up || key == KeyCode::Down) {
        std::vector<LineLayout> lines;
        int line_height = 1;
        build_layout_lines(lines, line_height);

        int caret_index = m_sel_end;
        if (has_selection && !shift && (key == KeyCode::Home || key == KeyCode::Up)) {
            caret_index = std::min(m_sel_start, m_sel_end);
        } else if (has_selection && !shift && (key == KeyCode::End || key == KeyCode::Down)) {
            caret_index = std::max(m_sel_start, m_sel_end);
        }

        if (key == KeyCode::Home) {
            int target = 0;
            if (!ctrl && !lines.empty()) {
                for (const auto& line : lines) {
                    if (caret_index >= line.start_idx && caret_index <= line.end_idx) {
                        target = line.start_idx;
                        break;
                    }
                }
            }
            m_vertical_nav_active = false;
            apply_cursor_move(target);
        } else if (key == KeyCode::End) {
            int target = len;
            if (!ctrl && !lines.empty()) {
                for (const auto& line : lines) {
                    if (caret_index >= line.start_idx && caret_index <= line.end_idx) {
                        target = line.end_idx;
                        break;
                    }
                }
            }
            m_vertical_nav_active = false;
            apply_cursor_move(target);
        } else if (key == KeyCode::Up || key == KeyCode::Down) {
            if (!lines.empty()) {
                IntPoint caret_pos = cursor_local_position(caret_index, lines);
                if (!m_vertical_nav_active) {
                    m_vertical_nav_x = caret_pos.x;
                    m_vertical_nav_active = true;
                }

                int target_y = (key == KeyCode::Up) ? (caret_pos.y - line_height) : (caret_pos.y + line_height);
                int target_index = cursor_index_from_local_point({m_vertical_nav_x, target_y}, lines, line_height);
                apply_cursor_move(target_index);
            }
        }
    } else if (key == KeyCode::Left) {
        int target = m_sel_end;
        if (has_selection && !shift) {
            target = std::min(m_sel_start, m_sel_end);
        } else if (target > 0) {
            target = ctrl ? word_start_at(target) : (target - 1);
        }
        m_vertical_nav_active = false;
        apply_cursor_move(target);
    } else if (key == KeyCode::Right) {
        int target = m_sel_end;
        if (has_selection && !shift) {
            target = std::max(m_sel_start, m_sel_end);
        } else if (target < len) {
            target = ctrl ? word_end_at(target) : (target + 1);
        }
        m_vertical_nav_active = false;
        apply_cursor_move(target);
    }

    if (handled) {
        if (old_sel_start != m_sel_start || old_sel_end != m_sel_end) {
            invalidate_visual();
        }
        return true;
    }

    return false;
}

bool Label::is_scrollable() const {
    return m_selecting;
}

void Label::measure(int parent_w, int parent_h) {
    int mw = 0, mh = 0;
    if (m_font) {
        int wrap_w = -1;
        if (m_should_wrap) wrap_w = parent_w;

        m_font->measure_multiline(m_text, mw, mh, wrap_w);
    }

    if (m_width == static_cast<int>(WidgetSizePolicy::MatchParent))
        mw = parent_w;
    else if (m_width > 0)
        mw = m_width;

    if (m_height == static_cast<int>(WidgetSizePolicy::MatchParent))
        mh = parent_h;
    else if (m_height > 0)
        mh = m_height;

    m_measured_size = {0, 0, mw, mh};
}

bool Label::has_running_animations() const {
    return Widget::has_running_animations() || m_scroll_anim.running() || m_selecting;
}

}  // namespace Izo
