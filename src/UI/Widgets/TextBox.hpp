#pragma once

#include "UI/Widgets/Widget.hpp"
#include "Graphics/Color.hpp"
#include "Motion/Animator.hpp"

#include <string>
#include <functional>

namespace Izo {

class Font;

class TextBox : public Widget {
public:
    TextBox(const std::string& placeholder);

    void set_text(const std::string& t);
    const std::string& text() const { return m_text_buffer; }
    void set_placeholder(const std::string& placeholder) { m_placeholder = placeholder; }
    void set_on_change(std::function<void(const std::string&)> callback) { m_on_change = callback; }
    void set_on_submit(std::function<void(const std::string&)> callback) { m_on_submit = callback; }
    void clear() { set_text(""); }

    void draw_content(Painter& painter) override;
    void update() override;
    bool on_touch_event(IntPoint point, bool down) override;
    bool on_key(KeyCode key) override;
    void measure(int parent_w, int parent_h) override;

private:
    int get_cursor_index(int lx);
    void ensure_cursor_visible();
    int find_word_start(int pos);
    int find_word_end(int pos);

    std::string m_text_buffer;
    std::string m_placeholder;

    Animator<Color> m_border_anim;

    int m_scroll_x = 0;
    int m_sel_start = 0;
    int m_sel_end = 0;
    bool m_is_dragging = false;
    float m_cursor_timer = 0.0f;
    bool m_cursor_visible = true;
    std::function<void(const std::string&)> m_on_change;
    std::function<void(const std::string&)> m_on_submit;

    static std::string s_clipboard;
};

}
