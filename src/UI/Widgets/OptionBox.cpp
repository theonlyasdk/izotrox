#include "UI/Widgets/OptionBox.hpp"

#include <algorithm>

#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "Core/ViewManager.hpp"
#include "Geometry/Primitives.hpp"
#include "Graphics/Dialog.hpp"
#include "Graphics/Font.hpp"
#include "UI/Widgets/OptionItem.hpp"

namespace Izo {

constexpr int DIALOG_MIN_WIDTH = 400;
constexpr int DIALOG_PADDING = 10;
constexpr int MARGIN_FROM_EDGE = 40;

class OptionsDialog : public Dialog {
   public:
    OptionsDialog(OptionBox* parent, const IntRect& start, int current_idx, std::function<void(int)> callback)
        : m_start(start), m_selected(current_idx), m_callback(callback) {
        m_options = parent->options();
        m_parent = parent;
        m_focusable = true;

        auto animation_variant = ThemeDB::the().get<OptionBox::AnimationVariant>("WidgetParams", "OptionBox.AnimationVariant", OptionBox::AnimationVariant::ExpandVertical);
        parent->set_anim_variant(animation_variant);

        set_padding(DIALOG_PADDING);

        for (int i = 0; i < (int)m_options->size(); ++i) {
            auto item = std::make_unique<OptionItem>(m_options->at(i), i, [this](int idx) {
                m_callback(idx);
                close();
            });
            if (i == m_selected) item->set_selected(true);
            add_child(std::move(item));
        }

        // Initial layout constants
        int win_w = Application::the().width();
        int win_h = Application::the().height();
        int dialog_w = std::min(win_w - MARGIN_FROM_EDGE, DIALOG_MIN_WIDTH);

        // Measure children to determine height
        // We subtract padding from both sides
        measure(dialog_w - (DIALOG_PADDING * 2), win_h - MARGIN_FROM_EDGE);

        int dialog_h = std::min(win_h - MARGIN_FROM_EDGE, m_measured_size.h);

        switch (parent->anim_variant()) {
            case OptionBox::AnimationVariant::ExpandCenter:
                m_target = {(win_w - dialog_w) / 2, (win_h - dialog_h) / 2, dialog_w, dialog_h};
                break;
            case OptionBox::AnimationVariant::ExpandVertical:
                m_target = {start.x, (win_h - dialog_h) / 2, start.w, dialog_h};
                break;
        }

        set_bounds(m_target);

        // Setup animation
        auto duration = ThemeDB::the().get<int>("WidgetParams", "OptionBox.AnimationDuration", 300);
        auto easing = ThemeDB::the().get<Easing>("WidgetParams", "OptionBox.AnimationEasing", Easing::EaseOutQuart);
        m_dialog_anim.set_target(1.0f, duration, easing);
    }

    void update() override {
        Dialog::update();

        if (m_closing && !m_dialog_anim.running()) {
            ViewManager::the().dismiss_dialog();
        }
    }

    void draw_content(Painter& painter) override {
        float anim_progress = m_dialog_anim.value();
        if (anim_progress <= 0.0f) return;

        float old_alpha = painter.global_alpha();
        painter.set_global_alpha(old_alpha * anim_progress);

        // Interpolate bounds
        IntRect current{
            m_start.x + (int)((m_target.x - m_start.x) * anim_progress),
            m_start.y + (int)((m_target.y - m_start.y) * anim_progress),
            m_start.w + (int)((m_target.w - m_start.w) * anim_progress),
            m_start.h + (int)((m_target.h - m_start.h) * anim_progress),
        };

        set_bounds(current);
        // We must remeasure and layout children since bounds are changing every frame
        measure(current.w - (DIALOG_PADDING * 2), current.h - (DIALOG_PADDING * 2));
        layout();

        auto roundness = ThemeDB::the().get<int>("WidgetParams", "Widget.Roundness", 12);
        auto color_bg = ThemeDB::the().get<Color>("Colors", "OptionBox.Background", Color(100));
        auto color_border = ThemeDB::the().get<Color>("Colors", "OptionBox.Border", Color(200));

        int outer_radius = roundness < DIALOG_PADDING - roundness ? roundness : roundness + DIALOG_PADDING;

        painter.fill_rounded_rect(current, outer_radius, color_bg);
        painter.draw_rounded_rect(current, outer_radius, color_border);

        Dialog::draw_content(painter);

        painter.set_global_alpha(old_alpha);
    }

    bool on_touch(IntPoint point, bool down, bool captured) override {
        if (m_closing) return true;

        bool inside = global_bounds().contains(point);

        if (down && !m_prev_touch_down) {
            m_touch_started_outside = !inside;
        }

        if (!down && m_prev_touch_down) {
            if (!inside && m_touch_started_outside) {
                close();
                return true;
            }
        }

        return Dialog::on_touch(point, down, captured);
    }

    void close() override {
        if (m_closing) return;
        m_closing = true;

        auto duration = ThemeDB::the().get<int>("WidgetParams", "OptionBox.AnimationDuration", 300);
        auto easing = ThemeDB::the().get<Easing>("WidgetParams", "OptionBox.AnimationEasing", Easing::EaseOutQuart);
        m_dialog_anim.set_target(0.0f, duration, easing);
    }

   private:
    IntRect m_start, m_target;
    std::vector<std::string>* m_options;
    OptionBox* m_parent;
    int m_selected;
    bool m_closing = false;
    bool m_touch_started_outside = false;
    bool m_dimmer_touch = false;
    std::function<void(int)> m_callback;
};

OptionBox::OptionBox()
    : m_selected_index(0),
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
    int total_text_height = m_font ? m_font->height() : 20;

    for (const auto& option : m_options) {
        if (m_font) total_text_width = std::max(total_text_width, m_font->width(option));
    }

    m_measured_size.w = total_text_width + m_padding_left + m_padding_right + 40;  // +40 for arrow
    m_measured_size.h = total_text_height + m_padding_top + m_padding_bottom;
}

void OptionBox::update() {
    Widget::update();
    m_bg_anim.update(Application::the().delta());
}

void OptionBox::draw_content(Painter& painter) {
    auto roundness = ThemeDB::the().get<int>("WidgetParams", "Widget.Roundness", 12);
    auto color_border = ThemeDB::the().get<Color>("Colors", "OptionBox.Border", Color(200));
    auto color_text = ThemeDB::the().get<Color>("Colors", "OptionBox.Text", Color(255));
    auto color_arrow = ThemeDB::the().get<Color>("Colors", "OptionBox.Arrow", Color(200));

    IntRect bounds = global_bounds();
    painter.fill_rounded_rect(bounds, roundness, m_bg_anim.value());
    painter.draw_rounded_rect(bounds, roundness, color_border);

    painter.push_rounded_clip(bounds, roundness);

    if (m_font && m_selected_index >= 0 && m_selected_index < (int)m_options.size()) {
        int ty = bounds.y + (bounds.h - m_font->height()) / 2;
        m_font->draw_text(painter, {bounds.x + m_padding_left, ty}, m_options[m_selected_index], color_text);
    }
    painter.pop_clip();

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

            auto dialog = std::make_unique<OptionsDialog>(this, global_bounds(), m_selected_index, [this](int idx) {
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
