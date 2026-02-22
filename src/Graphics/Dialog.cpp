#include "Graphics/Dialog.hpp"
#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "UI/Layout/LinearLayout.hpp"

#include <algorithm>

namespace Izo {

Dialog::Dialog() : LinearLayout(Orientation::Vertical) {
    m_focusable = true;
    on_theme_update();
}

int Dialog::calculate_visual_roundness(int base_roundness, int padding) {
    if (base_roundness < 0) base_roundness = 0;
    if (padding < 0) padding = 0;
    return base_roundness < padding - base_roundness ? base_roundness : base_roundness + padding;
}

int Dialog::visual_roundness() const {
    int max_padding = std::max({m_padding_left, m_padding_right, m_padding_top, m_padding_bottom});
    return calculate_visual_roundness(m_dialog_base_roundness, max_padding);
}

void Dialog::draw_content(Painter& painter) {
    LinearLayout::draw_content(painter);
}

void Dialog::run_layout_pass() {
    int parent_w = std::max(1, static_cast<int>(Application::the().width()));
    int parent_h = std::max(1, static_cast<int>(Application::the().height()));
    bool parent_changed = parent_w != m_last_parent_w || parent_h != m_last_parent_h;
    bool needs_layout = subtree_layout_dirty() || parent_changed;

    if (!needs_layout) {
        return;
    }

    m_last_parent_w = parent_w;
    m_last_parent_h = parent_h;

    if (m_auto_layout_centered) {
        measure(parent_w, parent_h);
        int w = std::clamp(m_measured_size.w, 1, parent_w);
        int h = std::clamp(m_measured_size.h, 1, parent_h);

        if (m_dialog_bounds.w > 0) w = std::clamp(m_dialog_bounds.w, 1, parent_w);
        if (m_dialog_bounds.h > 0) h = std::clamp(m_dialog_bounds.h, 1, parent_h);

        int x = (parent_w - w) / 2;
        int y = (parent_h - h) / 2;
        if (m_dialog_bounds.w > 0 || m_dialog_bounds.h > 0) {
            x = std::clamp(m_dialog_bounds.x, 0, std::max(0, parent_w - w));
            y = std::clamp(m_dialog_bounds.y, 0, std::max(0, parent_h - h));
        }

        set_bounds({x, y, w, h});
    } else if (m_bounds.w <= 0 || m_bounds.h <= 0) {
        set_bounds({0, 0, parent_w, parent_h});
    }

    measure(std::max(1, m_bounds.w), std::max(1, m_bounds.h));
    layout();
    clear_layout_dirty_subtree();
    invalidate_visual();
}

void Dialog::update() {
    run_layout_pass();
    LinearLayout::update();
    run_layout_pass();
    m_dialog_anim.update(Application::the().delta());
}

void Dialog::on_theme_update() {
    LinearLayout::on_theme_update();
    m_animation_duration_ms = ThemeDB::the().get<int>("WidgetParams", "Dialog.AnimationDuration", 300);
    m_animation_easing = ThemeDB::the().get<Easing>("WidgetParams", "Dialog.AnimationEasing", Easing::EaseOutQuart);
    m_dialog_base_roundness = ThemeDB::the().get<int>("WidgetParams", "Widget.Roundness", 12);
    m_dialog_color_bg = ThemeDB::the().get<Color>("Colors", "Dialog.Background", Color(40, 40, 40, 240));
    m_dialog_color_border = ThemeDB::the().get<Color>("Colors", "Dialog.Border", Color(100, 100, 100, 180));
    m_dialog_color_shadow = ThemeDB::the().get<Color>("Colors", "Dialog.Shadow", Color(0, 0, 0, 96));
}

void Dialog::open() {
    if (m_dialog_visible) return;
    m_dialog_visible = true;
    
    m_dialog_anim.set_target(1.0f, m_animation_duration_ms, m_animation_easing);
}

void Dialog::close() {
    if (!m_dialog_visible) return;
    
    m_dialog_anim.set_target(0.0f, m_animation_duration_ms, m_animation_easing);
}

} 
