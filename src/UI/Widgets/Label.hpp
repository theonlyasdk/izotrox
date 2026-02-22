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
    bool m_cursor_visible = false;
    float m_cursor_timer = 0.0f;
    int m_cursor_blink_speed_ms = 500;
    Color m_color_selection{100, 100, 100, 120};
    Color m_color_cursor{255, 255, 255};
};

}
