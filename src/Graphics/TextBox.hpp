// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "Widget.hpp"
#include "Font.hpp"
#include "Color.hpp"
#include "Animator.hpp"
#include <string>

namespace Izo {

class TextBox : public Widget {
public:
    TextBox(const std::string& placeholder, Font* font);

    void set_text(const std::string& t);
    const std::string& text() const;

    void draw_content(Painter& painter) override;
    void update() override;
    bool on_touch_event(int local_x, int local_y, bool down) override;
    bool on_key(int key) override;
    void measure(int parent_w, int parent_h) override;

private:
    int get_cursor_index(int lx);
    void ensure_cursor_visible();

    std::string m_text_buffer;
    std::string m_placeholder;
    Font* m_font;
    
    Animator<Color> m_border_anim;
    
    int m_scroll_x = 0;
    int m_sel_start = 0;
    int m_sel_end = 0;
    bool m_is_dragging = false;
};

} // namespace Izo