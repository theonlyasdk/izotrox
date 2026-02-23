#include "UI/Widgets/OptionBox.hpp"

#include <algorithm>
#include <utility>

#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "Core/ViewManager.hpp"
#include "Geometry/Primitives.hpp"
#include "Graphics/Dialog.hpp"
#include "Graphics/Font.hpp"
#include "UI/Widgets/OptionItem.hpp"

namespace Izo {

constexpr int kDialogPadding = 10;
constexpr int kMarginFromEdge = 40;

class OptionsDialog : public Dialog {
   public:
    OptionsDialog(OptionBox* parent, const IntRect& start, int current_idx, std::function<void(int)> callback)
        : m_parent(parent), m_start(start), m_selected(current_idx), 
        m_callback(std::move(callback)) {
        set_widget_type("OptionsDialog");

        set_focusable(true);
        set_padding(kDialogPadding);

        m_variant = ThemeDB::the().get<OptionBox::AnimationVariant>(
            "WidgetParams", "OptionBox.AnimationVariant", 
            OptionBox::AnimationVariant::ExpandVertical);

        switch (m_variant) {
            case OptionBox::AnimationVariant::ExpandCenter:
            case OptionBox::AnimationVariant::ExpandVertical:
                set_dim_background_on_open(true);
                break;
            case OptionBox::AnimationVariant::ExpandDropdown:
                set_dim_background_on_open(false);
                break;
        }
        parent->set_anim_variant(m_variant);

        const auto& options = *parent->options();
        for (int i = 0; i < (int)options.size(); ++i) {
            auto item = std::make_unique<OptionItem>(options[i], i, [this](int idx) {
                m_callback(idx);
                close();
            });
            if (i == m_selected) 
                item->set_selected(true);
            add_child(std::move(item));
        }

        refresh_target_bounds();

        set_bounds(m_target);
        measure(std::max(1, m_target.w - (kDialogPadding * 2)), std::max(1, m_target.h - (kDialogPadding * 2)));
        layout();

        m_dialog_anim.set_target(1.0f, m_animation_duration_ms, m_animation_easing);

        on_theme_update();
    }

    void update() override {
        Dialog::update();
    }

    void draw_content(Painter& painter) override {
        refresh_target_bounds();

        float anim_progress = m_dialog_anim.value();
        if (anim_progress <= 0.0f) return;

        float old_alpha = painter.global_alpha();
        painter.set_global_alpha(old_alpha * anim_progress);

        IntRect current = m_target;
        if (m_variant == OptionBox::AnimationVariant::ExpandDropdown) {
            current.h = std::max(1, static_cast<int>(m_target.h * anim_progress));
            if (m_dropdown_opens_up) {
                current.y = m_target.bottom() - current.h;
            }
        } else {
            current = {
                m_start.x + (int)((m_target.x - m_start.x) * anim_progress),
                m_start.y + (int)((m_target.y - m_start.y) * anim_progress),
                m_start.w + (int)((m_target.w - m_start.w) * anim_progress),
                m_start.h + (int)((m_target.h - m_start.h) * anim_progress),
            };
        }

        set_bounds(current);
        measure(std::max(1, current.w - (kDialogPadding * 2)), std::max(1, current.h - (kDialogPadding * 2)));
        layout();

        int outer_radius = calculate_visual_roundness(m_roundness, kDialogPadding);

        painter.fill_rounded_rect(current, outer_radius, m_color_bg);
        painter.draw_rounded_rect(current, outer_radius, m_color_border);

        painter.push_rounded_clip(current, outer_radius);
        Dialog::draw_content(painter);
        painter.pop_clip();

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
        m_dialog_anim.set_target(0.0f, m_animation_duration_ms, m_animation_easing);
    }

    void on_theme_update() override {
        Dialog::on_theme_update();
        m_roundness = ThemeDB::the().get<int>("WidgetParams", "Widget.Roundness", 12);
        m_color_bg = ThemeDB::the().get<Color>("Colors", "OptionBox.Background", Color(100));
        m_color_border = ThemeDB::the().get<Color>("Colors", "OptionBox.Border", Color(200));
        m_animation_duration_ms = ThemeDB::the().get<int>("WidgetParams", "OptionBox.AnimationDuration", 300);
        m_animation_easing = ThemeDB::the().get<Easing>("WidgetParams", "OptionBox.AnimationEasing", Easing::EaseOutQuart);
    }

   private:
    void refresh_target_bounds() {
        IntRect anchor = m_start;
        if (m_parent) {
            anchor = m_parent->global_bounds();
        }

        int win_w = static_cast<int>(Application::the().width());
        int win_h = static_cast<int>(Application::the().height());
        int dialog_w = std::min(win_w - kMarginFromEdge, 400);
        int content_w_for_measure = dialog_w - (kDialogPadding * 2);

        if (m_variant == OptionBox::AnimationVariant::ExpandVertical ||
            m_variant == OptionBox::AnimationVariant::ExpandDropdown) {
            content_w_for_measure = std::max(1, anchor.w - (kDialogPadding * 2));
        }

        measure(std::max(1, content_w_for_measure), std::max(1, win_h - kMarginFromEdge));
        int dialog_h = std::min(win_h - kMarginFromEdge, m_measured_size.h);

        switch (m_variant) {
            case OptionBox::AnimationVariant::ExpandCenter:
                m_target = {(win_w - dialog_w) / 2, (win_h - dialog_h) / 2, dialog_w, dialog_h};
                break;
            case OptionBox::AnimationVariant::ExpandVertical: {
                int x = std::clamp(anchor.x, 0, std::max(0, win_w - anchor.w));
                m_target = {x, (win_h - dialog_h) / 2, anchor.w, dialog_h};
                break;
            }
            case OptionBox::AnimationVariant::ExpandDropdown: {
                int dropdown_w = anchor.w;
                int margin = kMarginFromEdge / 2;
                int x = std::clamp(anchor.x, margin, std::max(margin, win_w - margin - dropdown_w));
                int max_h_below = std::max(1, win_h - margin - anchor.bottom());
                int max_h_above = std::max(1, anchor.y - margin);

                bool open_up = false;
                if (max_h_below < dialog_h && max_h_above > max_h_below) {
                    open_up = true;
                }

                if (open_up) {
                    int dropdown_h = std::min(dialog_h, max_h_above);
                    int y = anchor.y - dropdown_h;
                    m_target = {x, y, dropdown_w, dropdown_h};
                    m_start = {x, anchor.y, dropdown_w, 0};
                    m_dropdown_opens_up = true;
                } else {
                    int dropdown_h = std::min(dialog_h, max_h_below);
                    m_target = {x, anchor.bottom(), dropdown_w, dropdown_h};
                    m_start = {x, anchor.bottom(), dropdown_w, 0};
                    m_dropdown_opens_up = false;
                }
                break;
            }
        }
    }

    OptionBox* m_parent = nullptr;
    IntRect m_start, m_target;
    OptionBox::AnimationVariant m_variant{OptionBox::AnimationVariant::ExpandVertical};
    int m_selected  = -1;
    int m_roundness = 12;
    int m_animation_duration_ms = 300;
    bool m_dropdown_opens_up = false;
    bool m_touch_started_outside = false;
    std::function<void(int)> m_callback;
    Color m_color_bg{100, 100, 100};
    Color m_color_border{200, 200, 200};
    Easing m_animation_easing = Easing::EaseOutQuart;
};

OptionBox::OptionBox() : m_selected_index(0), m_bg_anim(Color(200)) {
    set_focusable(true);
    set_padding_ltrb(12, 8, 12, 8);
    on_theme_update();
}

void OptionBox::on_theme_update() {
    Widget::on_theme_update();
    m_roundness = ThemeDB::the().get<int>("WidgetParams", "Widget.Roundness", 12);
    m_color_background = ThemeDB::the().get<Color>("Colors", "OptionBox.Background", Color(100));
    m_color_active = ThemeDB::the().get<Color>("Colors", "OptionBox.Active", Color(100));
    m_color_border = ThemeDB::the().get<Color>("Colors", "OptionBox.Border", Color(200));
    m_color_text = ThemeDB::the().get<Color>("Colors", "OptionBox.Text", Color(255));
    m_color_arrow = ThemeDB::the().get<Color>("Colors", "OptionBox.Arrow", Color(200));
    m_animation_duration_ms = ThemeDB::the().get<int>("WidgetParams", "OptionBox.AnimationDuration", 300);
    m_animation_easing = ThemeDB::the().get<Easing>("WidgetParams", "OptionBox.AnimationEasing", Easing::EaseOutQuart);
    m_bg_anim.snap_to(m_color_background);
    invalidate_visual();
}

void OptionBox::add_option(const std::string& option) {
    m_options.push_back(option);
}

void OptionBox::set_options(const std::vector<std::string>& options) {
    m_options = options;
    if (m_selected_index >= (int)m_options.size()) {
        m_selected_index = m_options.empty() ? -1 : 0;
    }
    invalidate_layout();
}

void OptionBox::select(int index) {
    if (index >= 0 && index < (int)m_options.size()) {
        if (m_selected_index == index) return;
        m_selected_index = index;
        invalidate_visual();
    }
}

void OptionBox::set_anim_variant(AnimationVariant variant) {
    if (m_anim_variant == variant) return;
    m_anim_variant = variant;
    invalidate_layout();
}

void OptionBox::measure(int parent_w, int parent_h) {
    int total_text_width = 0;
    int total_text_height = m_font ? m_font->height() : 20;

    for (const auto& option : m_options) {
        if (m_font) total_text_width = std::max(total_text_width, m_font->width(option));
    }

    m_measured_size.w = total_text_width + m_padding.left + m_padding.right + 40;  // +40 for arrow
    m_measured_size.h = total_text_height + m_padding.top + m_padding.bottom;
}

void OptionBox::update() {
    Color old_bg = m_bg_anim.value();
    bool was_running = m_bg_anim.running();
    Widget::update();
    m_bg_anim.update(Application::the().delta());
    if (was_running || m_bg_anim.running() || old_bg.as_argb() != m_bg_anim.value().as_argb()) {
        invalidate_visual();
    }
}

void OptionBox::draw_content(Painter& painter) {
    IntRect bounds = global_bounds();
    painter.fill_rounded_rect(bounds, m_roundness, m_bg_anim.value());
    painter.draw_rounded_rect(bounds, m_roundness, m_color_border);

    painter.push_rounded_clip(bounds, m_roundness);

    if (m_font && m_selected_index >= 0 && m_selected_index < (int)m_options.size()) {
        int ty = bounds.y + (bounds.h - m_font->height()) / 2;
        m_font->draw_text(painter, {bounds.x + m_padding.left, ty}, m_options[m_selected_index], m_color_text);
    }
    painter.pop_clip();

    int ax = bounds.x + bounds.w - 20;
    int ay = bounds.y + bounds.h / 2;
    painter.draw_line({ax - 4, ay - 2}, {ax, ay + 2}, m_color_arrow);
    painter.draw_line({ax, ay + 2}, {ax + 4, ay - 2}, m_color_arrow);
}

bool OptionBox::on_touch_event(IntPoint point, bool down) {
    if (!content_box().contains(point)) {
        m_pressed = false;
        m_bg_anim.set_target(m_color_background, 200);
        invalidate_visual();
        return false;
    }

    if (down) {
        m_pressed = true;
        m_bg_anim.set_target(m_color_active, 100);
        invalidate_visual();
        return true;
    }

    if (!m_pressed) return true;

    m_pressed = false;
    m_bg_anim.set_target(m_color_background, 200);
    invalidate_visual();

    auto dialog = std::make_unique<OptionsDialog>(this, global_bounds(), m_selected_index, [this](int idx) {
        select(idx);
        if (m_on_change)
            m_on_change(idx, m_options[idx]);
    });
    ViewManager::the().open_dialog(std::move(dialog));
    return true;
}

bool OptionBox::has_running_animations() const {
    return Widget::has_running_animations() || m_bg_anim.running();
}

}  // namespace Izo
