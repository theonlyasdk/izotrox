#include "Graphics/Dialog.hpp"
#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "UI/Layout/LinearLayout.hpp"

namespace Izo {

Dialog::Dialog() : LinearLayout(Orientation::Vertical) {
    m_focusable = true;
}

void Dialog::draw_content(Painter& painter) {
    LinearLayout::draw_content(painter);
}

void Dialog::update() {
    LinearLayout::update();
    m_dialog_anim.update(Application::the().delta());
}

void Dialog::open() {
    if (m_dialog_visible) return;
    m_dialog_visible = true;
    
    int duration = ThemeDB::the().get<int>("WidgetParams", "Dialog.AnimationDuration", 300);
    Easing easing = ThemeDB::the().get<Easing>("WidgetParams", "Dialog.AnimationEasing", Easing::EaseOutQuart);
    m_dialog_anim.set_target(1.0f, duration, easing);
}

void Dialog::close() {
    if (!m_dialog_visible) return;
    
    int duration = ThemeDB::the().get<int>("WidgetParams", "Dialog.AnimationDuration", 300);
    Easing easing = ThemeDB::the().get<Easing>("WidgetParams", "Dialog.AnimationEasing", Easing::EaseOutQuart);
    m_dialog_anim.set_target(0.0f, duration, easing);
}

} 
