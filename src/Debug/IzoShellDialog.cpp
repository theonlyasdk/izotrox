#include "Debug/IzoShellDialog.hpp"

#include <algorithm>

#include "Debug/IzoShell.hpp"
#include "Graphics/Painter.hpp"
#include "UI/Widgets/TextBox.hpp"
#include "UI/Widgets/Toast.hpp"

namespace Izo {

namespace {
constexpr int kDialogPadding = 18;
constexpr int kTextBoxWidth = 620;
constexpr int kShadowBlurRadius = 16;
constexpr IntPoint kShadowOffset{3, 12};
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
    m_input->set_width(kTextBoxWidth);
    m_input->set_on_submit([this](const std::string& text) {
        auto result = IzoShell::the().execute(text);
        if (result.ok) {
            close();
            return;
        }

        ToastManager::the().show(result.error, 2200);
    });
    add_child(std::move(input));

    if (m_input) {
        m_input->set_focused(true);
    }
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
    Dialog::update();
}

bool IzoShellDialog::on_touch(IntPoint point, bool down, bool captured) {
    if (!down && !global_bounds().contains(point)) {
        close();
        return true;
    }

    return Dialog::on_touch(point, down, captured);
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
