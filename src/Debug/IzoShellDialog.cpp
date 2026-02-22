#include "Debug/IzoShellDialog.hpp"

#include <algorithm>
#include <cmath>

#include "Core/Application.hpp"
#include "Debug/IzoShell.hpp"
#include "Graphics/Painter.hpp"
#include "UI/Widgets/Label.hpp"
#include "UI/Widgets/TextBox.hpp"
#include "UI/Widgets/Toast.hpp"

namespace Izo {

namespace {
constexpr int kDialogPadding = 18;
constexpr int kDialogEdgePadding = 24;
constexpr int kDialogMinWidth = 360;
constexpr int kShadowBlurRadius = 16;
constexpr IntPoint kShadowOffset{3, 12};
constexpr int kLayoutSpacingPx = 10;  // Matches LinearLayout spacing.
constexpr int kDialogResizeDurationMs = 180;
}  // namespace

IzoShellDialog::IzoShellDialog() {
    m_focusable = true;
    set_show_focus_indicator(false);
    m_dialog_visible = true;
    m_dialog_anim.snap_to(1.0f);

    set_auto_layout_centered(true);
    set_padding(kDialogPadding);
    set_width(WidgetSizePolicy::WrapContent);
    set_height(WidgetSizePolicy::WrapContent);

    auto input = std::make_unique<TextBox>("Run IzoShell command");
    m_input = input.get();
    m_input->set_focusable(true);
    m_input->set_show_focus_indicator(false);
    m_input->set_width(WidgetSizePolicy::MatchParent);
    m_input->set_on_submit([this](const std::string& text) {
        if (text.empty()) {
            set_output_text("Command cannot be empty", true);
            ToastManager::the().show("Command cannot be empty", 2200);
            return;
        }

        auto result = IzoShell::the().execute(text);
        if (result.ok) {
            std::string output = result.output;
            if (output.empty()) {
                output = "Command executed.";
            }
            set_output_text(output, false);
            if (m_input) {
                m_input->clear();
            }
            return;
        }

        set_output_text(result.error, true);
        ToastManager::the().show(result.error, 2200);
    });
    add_child(std::move(input));

    auto output_label = std::make_unique<Label>("");
    m_output_label = output_label.get();
    m_output_label->set_width(WidgetSizePolicy::MatchParent);
    m_output_label->set_wrap(true);
    m_output_label->set_focusable(false);
    m_output_label->set_color_variant(ColorVariant::Secondary);
    m_output_label->set_height(0);
    m_output_label->hide();
    add_child(std::move(output_label));

    if (m_input) {
        m_input->set_focused(true);
    }
}

void IzoShellDialog::set_output_text(const std::string& text, bool is_error) {
    if (!m_output_label) return;

    m_output_text = text;
    m_output_visible = !m_output_text.empty();

    m_output_label->set_text(m_output_text);
    m_output_label->set_color_variant(is_error ? ColorVariant::Error : ColorVariant::Secondary);

    if (m_output_visible) {
        m_output_label->show();
    } else {
        m_output_label->hide();
    }

    invalidate_layout();
}

void IzoShellDialog::draw_content(Painter& painter) {
    IntRect panel_bounds = global_bounds();
    int outer_radius = visual_roundness();

    painter.draw_drop_shadow_rect(panel_bounds, kShadowBlurRadius, m_dialog_color_shadow, outer_radius, kShadowOffset);

    painter.fill_rounded_rect(panel_bounds, outer_radius, m_dialog_color_bg);
    painter.draw_rounded_rect(panel_bounds, outer_radius, m_dialog_color_border, 1);

    Color accent = m_dialog_color_border;
    accent.a = std::min<uint8_t>(accent.a, 26);
    painter.fill_rect({panel_bounds.x + 24, panel_bounds.y + 16, panel_bounds.w - 48, 1}, accent);

    painter.push_rounded_clip(panel_bounds, outer_radius);
    Dialog::draw_content(painter);
    painter.pop_clip();
}

void IzoShellDialog::update() {
    const float delta_ms = Application::the().delta();
    const int screen_w = std::max(1, static_cast<int>(Application::the().width()));
    const int screen_h = std::max(1, static_cast<int>(Application::the().height()));
    const int max_dialog_h = std::max(120, screen_h - (kDialogEdgePadding * 2));

    int target_w = screen_w - (kDialogEdgePadding * 2);
    target_w = std::max(1, target_w);
    if (target_w < kDialogMinWidth && screen_w > kDialogMinWidth) {
        target_w = kDialogMinWidth;
    }
    target_w = std::min(target_w, screen_w);
    int content_w = std::max(1, target_w - m_padding_left - m_padding_right);

    int input_h = 40;
    if (m_input) {
        m_input->measure(content_w, max_dialog_h);
        input_h = std::max(1, m_input->measured_height());
    }

    int output_h = 0;
    if (m_output_label && m_output_visible) {
        m_output_label->show();
        m_output_label->measure(content_w, max_dialog_h);
        output_h = std::max(1, m_output_label->measured_height());
    } else if (m_output_label) {
        m_output_label->hide();
    }

    int target_h = m_padding_top + m_padding_bottom + input_h;
    if (output_h > 0) {
        target_h += kLayoutSpacingPx + output_h;
    }
    target_h = std::clamp(target_h, 1, max_dialog_h);

    if (!m_dialog_size_initialized) {
        m_dialog_size_initialized = true;
        m_dialog_target_height = static_cast<float>(target_h);
        m_dialog_height_anim.snap_to(m_dialog_target_height);
    } else if (std::abs(static_cast<float>(target_h) - m_dialog_target_height) > 0.5f) {
        m_dialog_target_height = static_cast<float>(target_h);
        m_dialog_height_anim.set_target(m_dialog_target_height, kDialogResizeDurationMs, Easing::EaseOutCubic);
    }

    m_dialog_height_anim.update(delta_ms);
    int animated_h = std::clamp(static_cast<int>(std::round(m_dialog_height_anim.value())), 1, max_dialog_h);
    int dialog_x = std::max(0, (screen_w - target_w) / 2);
    int dialog_y = std::max(0, (screen_h - animated_h) / 2);

    IntRect old_bounds = dialog_bounds();
    if (old_bounds.x != dialog_x || old_bounds.y != dialog_y || old_bounds.w != target_w || old_bounds.h != animated_h) {
        set_dialog_bounds({dialog_x, dialog_y, target_w, animated_h});
        invalidate_layout();
    } else if (m_dialog_height_anim.running()) {
        invalidate_layout();
    }

    if (m_dialog_height_anim.running()) {
        invalidate_visual();
    }

    Dialog::update();
}

bool IzoShellDialog::on_touch(IntPoint point, bool down, bool captured) {
    bool inside = global_bounds().contains(point);

    if (down && !m_prev_touch_down) {
        m_touch_started_outside = !inside;
    }

    if (!down && m_prev_touch_down) {
        if (!inside && m_touch_started_outside) {
            close();
            return true;
        }
        m_touch_started_outside = false;
    }

    return Dialog::on_touch(point, down, captured);
}

bool IzoShellDialog::has_running_animations() const {
    return Dialog::has_running_animations() || m_dialog_height_anim.running();
}

bool IzoShellDialog::on_key(KeyCode key) {
    if (key == KeyCode::Escape) {
        close();
        return true;
    }

    return Dialog::on_key(key);
}

void IzoShellDialog::close() {
    if (m_closing) return;
    m_closing = true;
    m_dialog_anim.snap_to(0.0f);
}

}  // namespace Izo
