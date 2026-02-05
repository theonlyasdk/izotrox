#include "Graphics/Dialog.hpp"
#include "Core/ThemeDB.hpp"

namespace Izo {

Dialog::Dialog() {
    m_focusable = true;
}

void Dialog::draw_content(Painter& painter) {
}

void Dialog::update() {
    Widget::update();
    m_dialog_anim.update(16.0f);
}

void Dialog::show_dialog() {
    if (m_dialog_visible) return;
    m_dialog_visible = true;
    
    int duration = ThemeDB::the().get<int>("Feel", "Dialog.AnimationDuration", 300);
    Easing easing = ThemeDB::the().get<Easing>("Feel", "Dialog.AnimationEasing", Easing::EaseOutQuart);
    m_dialog_anim.set_target(1.0f, duration, easing);
}

void Dialog::hide_dialog() {
    if (!m_dialog_visible) return;
    
    int duration = ThemeDB::the().get<int>("Feel", "Dialog.AnimationDuration", 300);
    Easing easing = ThemeDB::the().get<Easing>("Feel", "Dialog.AnimationEasing", Easing::EaseOutQuart);
    m_dialog_anim.set_target(0.0f, duration, easing);
}

} 
