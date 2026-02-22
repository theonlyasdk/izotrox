#pragma once

#include "UI/Widgets/Widget.hpp"
#include "Graphics/ColorVariant.hpp"
#include "Graphics/Color.hpp"
#include "UI/Enums.hpp"

#include <string>
#include <vector>

namespace Izo {

class Font;

class Label : public Widget {
public:
    Label(const std::string& text);
    void set_text(const std::string& text);
    const std::string& text() const { return m_text; }

    void draw_content(Painter& painter) override;
    void measure(int parent_w, int parent_h) override;
    bool on_touch_event(IntPoint point, bool down) override;
    bool on_key(KeyCode key) override;
    bool is_scrollable() const override;
    void on_theme_update() override;

    void set_wrap(bool wrap);
    
    void set_alignment(TextAlign align);
    TextAlign alignment() const { return m_alignment; }
    
    void set_color_variant(ColorVariant variant);
    ColorVariant color_variant() const { return m_color_variant; }

    void set_color(const Color& color);
    Color color() const;

    void update() override;
    bool has_running_animations() const override;

private:
    enum class SelectionDragMode {
        Character,
        Word,
        Paragraph,
    };

    struct LineLayout {
        std::string text;
        int start_idx = 0;
        int end_idx = 0;
        int x = 0;
        int y = 0;
        int width = 0;
    };

    void build_layout_lines(std::vector<LineLayout>& out_lines, int& line_height) const;
    int cursor_index_from_local_point(IntPoint local, const std::vector<LineLayout>& lines, int line_height) const;
    IntPoint cursor_local_position(int index, const std::vector<LineLayout>& lines) const;
    int word_start_at(int index) const;
    int word_end_at(int index) const;
    int paragraph_start_at(int index) const;
    int paragraph_end_at(int index) const;
    void apply_drag_selection(int current_index, int anchor_index, SelectionDragMode mode);
    void reset_cursor_blink();

    std::string m_text;
    bool m_should_wrap = false;
    TextAlign m_alignment = TextAlign::Left;
    ColorVariant m_color_variant = ColorVariant::Default;
    
    // Custom color
    bool m_has_custom_color = false;
    Color m_custom_color{0};

    // Marquee scrolling
    Animator<float> m_scroll_anim{0.0f};
    bool m_should_scroll = false;

    // Shift-selection
    int m_sel_start = 0;
    int m_sel_end = 0;
    bool m_selecting = false;
    bool m_touch_down = false;
    bool m_consume_touch = false;
    int m_drag_anchor = 0;
    int m_click_count = 0;
    long long m_last_click_ms = 0;
    IntPoint m_last_click_point{0, 0};
    SelectionDragMode m_drag_mode = SelectionDragMode::Character;
    bool m_vertical_nav_active = false;
    int m_vertical_nav_x = 0;
    bool m_cursor_visible = false;
    float m_cursor_timer = 0.0f;
    int m_cursor_blink_speed_ms = 500;
    Color m_color_selection{100, 100, 100, 120};
    Color m_color_cursor{255, 255, 255};
};

}
