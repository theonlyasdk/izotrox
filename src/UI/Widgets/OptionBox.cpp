#include "UI/Widgets/OptionBox.hpp"
#include "Graphics/Dialog.hpp"
#include "Core/ThemeDB.hpp"
#include "Core/ViewManager.hpp"
#include "Core/Application.hpp"
#include <algorithm>

namespace Izo {

class OptionsDialog : public Dialog {
public:
    OptionsDialog(const IntRect& start, const std::vector<std::string>& options, Font* font, int current_idx, std::function<void(int)> callback)
        : m_start(start), m_options(options), m_font(font), m_selected(current_idx), m_callback(callback) {
        m_focusable = true;
        
        int win_w = Application::the().width();
        int win_h = Application::the().height();
        
        int dw = std::min(win_w - 40, 400);
        int item_h = m_font ? m_font->height() + 20 : 40;
        int dh = std::min(win_h - 40, (int)m_options.size() * item_h + 20);
        
        m_target = { (win_w - dw) / 2, (win_h - dh) / 2, dw, dh };
        set_bounds({0, 0, win_w, win_h});

        int duration = ThemeDB::the().get<int>("Feel", "OptionBox.AnimationDuration", 300);
        Easing easing = ThemeDB::the().get<Easing>("Feel", "OptionBox.AnimationEasing", Easing::EaseOutQuart);
        m_anim.set_target(1.0f, duration, easing);
    }

    void update() override {
        Dialog::update();
        m_anim.update(Application::the().delta());
        if (m_closing && !m_anim.is_running()) {
            ViewManager::the().dismiss_dialog();
        }
    }

    void draw_content(Painter& painter) override {
        float t = m_anim.value();
        
        IntRect current;
        current.x = m_start.x + (int)((m_target.x - m_start.x) * t);
        current.y = m_start.y + (int)((m_target.y - m_start.y) * t);
        current.w = m_start.w + (int)((m_target.w - m_start.w) * t);
        current.h = m_start.h + (int)((m_target.h - m_start.h) * t);

        int roundness = ThemeDB::the().get<int>("Looks", "Widget.Roundness", 12);
        Color color_bg = ThemeDB::the().get<Color>("Colors", "OptionBox.ExpandedBackground", Color(100));
        Color color_border = ThemeDB::the().get<Color>("Colors", "OptionBox.Border", Color(200));
        Color color_highlight = ThemeDB::the().get<Color>("Colors", "OptionBox.Highlight", Color(200));
        Color color_text = ThemeDB::the().get<Color>("Colors", "OptionBox.Text", Color(255));
        
        painter.fill_rounded_rect(current, roundness, color_bg);
        painter.draw_rounded_rect(current, roundness, color_border);

        if (t > 0.5f && m_font) {
            float alpha = (t - 0.5f) * 2.0f;
            int item_h = m_font->height() + 20;
            
            painter.push_clip(current);
            for (int i = 0; i < (int)m_options.size(); ++i) {
                int iy = current.y + 10 + i * item_h;
                if (iy + item_h < current.y || iy > current.y + current.h) continue;

                if (i == m_selected || i == m_hover) {
                    color_highlight.a = (uint8_t)(color_highlight.a * alpha);
                    painter.fill_rect({current.x + 5, iy, current.w - 10, item_h}, color_highlight);
                }

                color_text.a = (uint8_t)(255 * alpha);
                m_font->draw_text(painter, {current.x + 20, iy + (item_h - m_font->height()) / 2}, m_options[i], color_text);
            }
            painter.pop_clip();
        }
    }

    bool on_touch_event(IntPoint point, bool down) override {
        if (m_closing) return true;

        float t = m_anim.value();
        IntRect current;
        current.x = m_start.x + (int)((m_target.x - m_start.x) * t);
        current.y = m_start.y + (int)((m_target.y - m_start.y) * t);
        current.w = m_start.w + (int)((m_target.w - m_start.w) * t);
        current.h = m_start.h + (int)((m_target.h - m_start.h) * t);

        if (!current.contains(point)) {
            if (!down) close();
            return true;
        }

        int item_h = m_font ? m_font->height() + 20 : 40;
        int idx = (point.y - current.y - 10) / item_h;

        if (idx >= 0 && idx < (int)m_options.size()) {
            if (down) {
                m_hover = idx;
            } else {
                m_callback(idx);
                close();
            }
        } else {
            m_hover = -1;
        }

        return true;
    }

    void close() {
        if (m_closing) return;
        m_closing = true;
        int duration = ThemeDB::the().get<int>("Feel", "OptionBox.AnimationDuration", 300);
        Easing easing = ThemeDB::the().get<Easing>("Feel", "OptionBox.AnimationEasing", Easing::EaseOutQuart);
        m_anim.set_target(0.0f, duration, easing);
    }

private:
    IntRect m_start, m_target;
    std::vector<std::string> m_options;
    Font* m_font;
    int m_selected;
    int m_hover = -1;
    Animator<float> m_anim{0.0f};
    bool m_closing = false;
    std::function<void(int)> m_callback;
};

OptionBox::OptionBox(Font* font)
    : m_font(font),
      m_bg_anim(ThemeDB::the().get<Color>("Colors", "OptionBox.Background", Color(200))) {
    m_focusable = true;
    set_padding_ltrb(12, 8, 12, 8);
}

void OptionBox::add_option(const std::string& option) {
    m_options.push_back(option);
}

void OptionBox::set_options(const std::vector<std::string>& options) {
    m_options = options;
    if (m_selected_index >= (int)m_options.size()) {
        m_selected_index = m_options.empty() ? -1 : 0;
    }
}

void OptionBox::set_selected_index(int index) {
    if (index >= 0 && index < (int)m_options.size()) {
        m_selected_index = index;
    }
}

std::string OptionBox::selected_value() const {
    if (m_selected_index >= 0 && m_selected_index < (int)m_options.size()) {
        return m_options[m_selected_index];
    }
    return "";
}

void OptionBox::measure(int parent_w, int parent_h) {
    int text_w = 0;
    int text_h = m_font ? m_font->height() : 20;
    for (const auto& opt : m_options) {
        if (m_font) text_w = std::max(text_w, m_font->width(opt));
    }
    m_measured_size.w = text_w + m_padding_left + m_padding_right + 30;
    m_measured_size.h = text_h + m_padding_top + m_padding_bottom;
}

void OptionBox::update() {
    Widget::update();
    m_bg_anim.update(Application::the().delta());
}

void OptionBox::draw_content(Painter& painter) {
    int roundness = ThemeDB::the().get<int>("Looks", "Widget.Roundness", 12);
    Color color_bg = ThemeDB::the().get<Color>("Colors", "OptionBox.Background", Color(100));
    Color color_border = ThemeDB::the().get<Color>("Colors", "OptionBox.Border", Color(200));
    Color color_highlight = ThemeDB::the().get<Color>("Colors", "OptionBox.Highlight", Color(200));
    Color color_text = ThemeDB::the().get<Color>("Colors", "OptionBox.Text", Color(255));
    Color color_arrow = ThemeDB::the().get<Color>("Colors", "OptionBox.Arrow", Color(200));
   
    IntRect b = m_bounds;
    painter.fill_rounded_rect(b, roundness, m_bg_anim.value());
    painter.draw_rounded_rect(b, roundness, color_border);

    if (m_font && m_selected_index >= 0 && m_selected_index < (int)m_options.size()) {
        int ty = b.y + (b.h - m_font->height()) / 2;
        m_font->draw_text(painter, {b.x + m_padding_left, ty}, m_options[m_selected_index], color_text);
    }

    int ax = b.x + b.w - 20;
    int ay = b.y + b.h / 2;
    painter.draw_line({ax - 4, ay - 2}, {ax, ay + 2}, color_arrow);
    painter.draw_line({ax, ay + 2}, {ax + 4, ay - 2}, color_arrow);
}

bool OptionBox::on_touch_event(IntPoint point, bool down) {
    Color color_bg = ThemeDB::the().get<Color>("Colors", "OptionBox.Background", Color(100));
    Color color_exp_bg = ThemeDB::the().get<Color>("Colors", "OptionBox.ExpandedBackground", Color(100));

    if (content_box().contains(point)) {
        if (down) {
            m_pressed = true;
            m_bg_anim.set_target(color_exp_bg, 100);
        } else if (m_pressed) {
            m_pressed = false;
            m_bg_anim.set_target(color_bg, 200);
            
            auto dialog = std::make_shared<OptionsDialog>(global_bounds(), m_options, m_font, m_selected_index, [this](int idx) {
                m_selected_index = idx;
                if (m_on_change) m_on_change(idx, m_options[idx]);
            });
            ViewManager::the().show_dialog(dialog);
        }
        return true;
    }
    m_pressed = false;
    m_bg_anim.set_target(color_bg, 200);
    return false;
}

} 
