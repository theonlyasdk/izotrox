#include "Graphics/Dialog.hpp"
#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "UI/Layout/LinearLayout.hpp"

namespace Izo {

Dialog::Dialog() : LinearLayout(Orientation::Vertical) {
    m_focusable = true;
    on_theme_update();
}

void Dialog::draw_content(Painter& painter) {
    LinearLayout::draw_content(painter);
}

void Dialog::update() {
    LinearLayout::update();
    m_dialog_anim.update(Application::the().delta());
}

void Dialog::on_theme_update() {
    LinearLayout::on_theme_update();
    m_animation_duration_ms = ThemeDB::the().get<int>("WidgetParams", "Dialog.AnimationDuration", 300);
    m_animation_easing = ThemeDB::the().get<Easing>("WidgetParams", "Dialog.AnimationEasing", Easing::EaseOutQuart);
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
