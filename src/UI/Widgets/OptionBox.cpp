#include "UI/Widgets/OptionBox.hpp"

#include <algorithm>

#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "Core/ViewManager.hpp"
#include "Geometry/Primitives.hpp"
#include "Graphics/Dialog.hpp"
#include "Graphics/Font.hpp"
#include "Input/Input.hpp"

namespace Izo {

constexpr int OPTION_LIST_ITEM_HEIGHT_PADDING = 20;
constexpr int OPTION_LIST_ITEM_GAP = 10;
constexpr int DIALOG_MIN_WIDTH = 400;
constexpr int DIALOG_MIN_HEIGHT = 600;
constexpr int DIALOG_PADDING = 10;
constexpr int MARGIN_FROM_EDGE = 40;

class OptionsDialog : public Dialog {
   public:
    OptionsDialog(const IntRect& start, const std::vector<std::string>& options, Font* font, int current_idx, std::function<void(int)> callback)
        : m_start(start), m_options(options), m_font(font), m_selected(current_idx), m_callback(callback) {
        m_focusable = true;

        int num_options = (int)m_options.size();
        int win_w = Application::the().width();
        int win_h = Application::the().height();

        int dialog_w = std::min(win_w - MARGIN_FROM_EDGE, DIALOG_MIN_WIDTH);
        int item_h = m_font->height() + OPTION_LIST_ITEM_HEIGHT_PADDING;
        int dialog_h = std::min(win_h - MARGIN_FROM_EDGE, num_options * item_h);

        IntRect target_rect{(win_w - dialog_w) / 2, (win_h - dialog_h) / 2, dialog_w, dialog_h};

        m_target = target_rect;
        set_bounds({0, 0, m_target.w, m_target.h});

        auto duration = ThemeDB::the().get<int>("Feel", "OptionBox.AnimationDuration", 300);
        auto easing = ThemeDB::the().get<Easing>("Feel", "OptionBox.AnimationEasing", Easing::EaseOutQuart);

        m_dialog_anim.set_target(1.0f, duration, easing);
    }

    void update() override {
        Dialog::update();
        m_dialog_anim.update(Application::the().delta());

        if (m_closing && !m_dialog_anim.running()) {
            ViewManager::the().dismiss_dialog();
        }
    }

    void draw_content(Painter& painter) override {
        float anim_progress = m_dialog_anim.value();

        int num_options = (int)m_options.size();
        int win_w = Application::the().width();
        int win_h = Application::the().height();

        int dialog_w = std::min(win_w - MARGIN_FROM_EDGE, DIALOG_MIN_WIDTH);
        int item_h = m_font->height() + OPTION_LIST_ITEM_HEIGHT_PADDING;
        int dialog_h = std::min(win_h - MARGIN_FROM_EDGE, num_options * item_h);

        IntRect dialog_bounds_after_anim{
            (win_w - dialog_w) / 2, (win_h - dialog_h) / 2, dialog_w, dialog_h + DIALOG_PADDING * 2,
        };

        IntRect current{
            m_start.x + (int)((dialog_bounds_after_anim.x - m_start.x) * anim_progress),
            m_start.y + (int)((dialog_bounds_after_anim.y - m_start.y) * anim_progress),
            m_start.w + (int)((dialog_bounds_after_anim.w - m_start.w) * anim_progress),
            m_start.h + (int)((dialog_bounds_after_anim.h - m_start.h) * anim_progress),
        };

        set_bounds(current);

        auto roundness = ThemeDB::the().get<int>("Looks", "Widget.Roundness", 12);
        auto color_bg = ThemeDB::the().get<Color>("Colors", "OptionBox.Background", Color(100));
        auto color_border = ThemeDB::the().get<Color>("Colors", "OptionBox.Border", Color(200));
        auto color_highlight = ThemeDB::the().get<Color>("Colors", "OptionBox.Highlight", Color(200));
        auto color_text = ThemeDB::the().get<Color>("Colors", "OptionBox.Text", Color(255));

        color_bg.a = (uint8_t)(color_bg.a * anim_progress);
        color_border.a = (uint8_t)(color_border.a * anim_progress);
        
        int outer_radius = roundness + DIALOG_PADDING;
        painter.fill_rounded_rect(current, outer_radius, color_bg);
        painter.draw_rounded_rect(current, outer_radius, color_border);

        if (anim_progress > 0.5f && m_font) {
            if (!m_dialog_anim.running()) {
                current.w = dialog_bounds_after_anim.w;
                current.h = dialog_bounds_after_anim.h;
            }

            float alpha = (anim_progress - 0.5f) * 2.0f;
            int item_h = m_font->height() + OPTION_LIST_ITEM_HEIGHT_PADDING;

            painter.push_clip(current);
            for (int i = 0; i < (int)m_options.size(); ++i) {
                int iy = current.y + DIALOG_PADDING + (i * item_h);

                if (iy + item_h < current.y || iy > current.y + current.h) 
                    continue;

                IntRect highlight_rect = {current.x, iy, current.w, item_h};
                highlight_rect.contract_horiz(DIALOG_PADDING);

                bool hovering = highlight_rect.contains(Input::the().touch_point());

                if (i == m_selected || hovering) {
                    color_highlight.a = (uint8_t)(color_highlight.a * alpha);
                    painter.fill_rounded_rect(highlight_rect, roundness, color_highlight);
                }

                color_text.a = (uint8_t)(255 * alpha);
                painter.push_rounded_clip(highlight_rect, roundness);
                m_font->draw_text(painter, {current.x + DIALOG_PADDING * 2, iy + (item_h - m_font->height()) / 2}, m_options[i], color_text);
                painter.pop_clip();
            }
            painter.pop_clip();
        }
    }

    bool on_touch_event(IntPoint point, bool down) override {
        if (m_closing) return true;

        // FIXME: Point passed to on_touch_event is transformed
        //        It's not clear that the point is transformed 
        //        causing unintended bugs. I just spent half an hour
        //        trying to fix the below logic before I realised
        //        the transformed point was the problem.
        IntPoint actual_touch_point = Input::the().touch_point();
        int touch_y = actual_touch_point.y;
        int item_height = m_font->height() + OPTION_LIST_ITEM_HEIGHT_PADDING;

        // We're just finding out i from y = dialog.y + padding + (i * item_height)
        int i = (touch_y - global_bounds().y - DIALOG_PADDING) / item_height;

        if (down) {
            if (!global_bounds().contains(actual_touch_point)) {
                close();
                return true;
            }
            if (i >= 0 && i < (int)m_options.size()) {
                m_callback(i);
                close();
            }
        }
        return true;
    }

    void close() override {
        if (m_closing) return;
        m_closing = true;

        auto duration = ThemeDB::the().get<int>("Feel", "OptionBox.AnimationDuration", 300);
        auto easing = ThemeDB::the().get<Easing>("Feel", "OptionBox.AnimationEasing", Easing::EaseOutQuart);

        m_dialog_anim.set_target(0.0f, duration, easing);
    }

   private:
    IntRect m_start, m_target;
    std::vector<std::string> m_options;
    Font* m_font;
    int m_selected;
    int m_hover = -1;
    bool m_closing = false;
    std::function<void(int)> m_callback;
};

OptionBox::OptionBox(Font* font)
    : m_font(font),
      m_selected_index(0),
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

void OptionBox::select(int index) {
    if (index >= 0 && index < (int)m_options.size()) {
        m_selected_index = index;
    }
}





void OptionBox::measure(int parent_w, int parent_h) {
    int total_text_width = 0;
    int total_text_height = m_font->height();

    for (const auto& option : m_options) {
        total_text_width = std::max(total_text_width, m_font->width(option));
    }

    m_measured_size.w = total_text_width + m_padding_left + m_padding_right;
    m_measured_size.h = total_text_height + m_padding_top + m_padding_bottom;
}

void OptionBox::update() {
    Widget::update();
    m_bg_anim.update(Application::the().delta());
}

void OptionBox::draw_content(Painter& painter) {
    auto roundness = ThemeDB::the().get<int>("Looks", "Widget.Roundness", 12);
    auto color_bg = ThemeDB::the().get<Color>("Colors", "OptionBox.Background", Color(100));
    auto color_border = ThemeDB::the().get<Color>("Colors", "OptionBox.Border", Color(200));
    auto color_highlight = ThemeDB::the().get<Color>("Colors", "OptionBox.Highlight", Color(200));
    auto color_text = ThemeDB::the().get<Color>("Colors", "OptionBox.Text", Color(255));
    auto color_arrow = ThemeDB::the().get<Color>("Colors", "OptionBox.Arrow", Color(200));

    IntRect bounds = global_bounds();
    painter.fill_rounded_rect(bounds, roundness, m_bg_anim.value());
    painter.draw_rounded_rect(bounds, roundness, color_border);

    if (m_font && m_selected_index >= 0 && m_selected_index < (int)m_options.size()) {
        int ty = bounds.y + (bounds.h - m_font->height()) / 2;
        m_font->draw_text(painter, {bounds.x + m_padding_left, ty}, m_options[m_selected_index], color_text);
    }

    int ax = bounds.x + bounds.w - 20;
    int ay = bounds.y + bounds.h / 2;
    painter.draw_line({ax - 4, ay - 2}, {ax, ay + 2}, color_arrow);
    painter.draw_line({ax, ay + 2}, {ax + 4, ay - 2}, color_arrow);
}

bool OptionBox::on_touch_event(IntPoint point, bool down) {
    auto color_bg = ThemeDB::the().get<Color>("Colors", "OptionBox.Background", Color(100));
    auto color_active = ThemeDB::the().get<Color>("Colors", "OptionBox.Active", Color(100));

    if (content_box().contains(point)) {
        if (down) {
            m_pressed = true;
            m_bg_anim.set_target(color_active, 100);
        } else if (m_pressed) {
            m_pressed = false;
            m_bg_anim.set_target(color_bg, 200);

            auto dialog = std::make_unique<OptionsDialog>(global_bounds(), m_options, m_font, m_selected_index, [this](int idx) {
                select(idx);
                if (m_on_change) 
                    m_on_change(idx, m_options[idx]);
            });
            ViewManager::the().open_dialog(std::move(dialog));
        }
        return true;
    }

    m_pressed = false;
    m_bg_anim.set_target(color_bg, 200);
    return false;
}

}  // namespace Izo
