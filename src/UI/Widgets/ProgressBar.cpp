#include <algorithm>
#include <cmath>

#include "UI/Widgets/ProgressBar.hpp"
#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "Graphics/Color.hpp"
#include "Graphics/Painter.hpp"
#include "Motion/Animator.hpp"

namespace Izo {

ProgressBar::ProgressBar(float progress) : m_value(progress) {
    set_focusable(true);
    set_height(20);
    on_theme_update();
}

ProgressBar::ProgressBar(bool indeterminate) : m_value(0) {
    set_height(20);
    set_focusable(true);
    on_theme_update();

    if (indeterminate) {
        m_type = ProgressBar::Type::Indeterminate;
        m_indeterminate_anim.set_target(1.0f, 1000, Easing::EaseInOutCubic);
        set_animation_variant(AnimationVariant::Variant2);
    }
    else m_type = ProgressBar::Type::Normal;
}

void ProgressBar::on_theme_update() {
    Widget::on_theme_update();
    m_roundness = ThemeDB::the().get<int>("WidgetParams", "Widget.Roundness", 6);
    m_color_bg = ThemeDB::the().get<Color>("Colors", "ProgressBar.Background", Color(100));
    m_color_fill = ThemeDB::the().get<Color>("Colors", "ProgressBar.Fill", Color(0, 255, 100));
    m_color_border = ThemeDB::the().get<Color>("Colors", "ProgressBar.Border", Color(200));
}

void ProgressBar::set_progress(float v) { 
    float new_progress = std::clamp(v, 0.0f, 1.0f);
    if (new_progress != m_value) {
        m_value = new_progress;
    }
}

void ProgressBar::set_animation_variant(ProgressBar::AnimationVariant variant) {
    m_variant = variant;

    switch (variant) {
        case AnimationVariant::Variant1:
            m_indeterminate_anim.set_loop(true);
            m_indeterminate_anim.set_loop_mode(AnimationLoopMode::ReverseOnLoop);
            break;
        case AnimationVariant::Variant2:
            m_indeterminate_anim.set_loop(true);
            m_indeterminate_anim.set_loop_mode(AnimationLoopMode::NoLoop);
            break;
    }
}

void ProgressBar::draw_normal(Painter& painter) {
    IntRect b = global_bounds();
    
    painter.fill_rounded_rect(b, m_roundness, m_color_bg);

    if (m_value > 0.0f) {
        int fill_w = static_cast<int>(b.w * m_value);
        if (fill_w > 0) {
            // We use clipping to ensure the fill respects the rounded corners of the background
            painter.push_clip({b.x, b.y, fill_w, b.h});
            painter.fill_rounded_rect(b, m_roundness, m_color_fill);
            painter.pop_clip();
        }
    }

    painter.draw_rounded_rect(b, m_roundness, m_color_border);
}

void ProgressBar::draw_indeterminate(Painter& painter) {
    static bool anim_swap = false;

    IntRect bounds = global_bounds();

    anim_swap = m_indeterminate_anim.loop_count() % 2 == 0;

    int fill_bar_width;
    int a_param_min_w = 1;
    int a_param_amp = bounds.w/2;

    if (anim_swap) {
        fill_bar_width = a_param_min_w + (a_param_amp * std::sin(m_indeterminate_anim.value()));
    } else {
        fill_bar_width = a_param_min_w + (a_param_amp * (1 - std::sin(m_indeterminate_anim.value())));
    }
    
    IntRect ind_anim_rect  = {};
    ind_anim_rect.h = bounds.h;
    ind_anim_rect.y = bounds.y;

    switch (m_variant) {
        case AnimationVariant::Variant1:
            ind_anim_rect.w = 100;
            ind_anim_rect.x = (int)(bounds.x + ((bounds.w - ind_anim_rect.w) * m_indeterminate_anim.value()));
            break;
        case AnimationVariant::Variant2:
            ind_anim_rect.w = fill_bar_width;
            ind_anim_rect.x = (int)(bounds.x - ind_anim_rect.w + ((bounds.w + ind_anim_rect.w) * m_indeterminate_anim.value()));
            break;
    }

    painter.fill_rounded_rect(bounds, m_roundness, m_color_bg);
    painter.draw_rounded_rect(bounds, m_roundness, m_color_border);
    // We use clipping to ensure the fill respects the rounded corners of the background
    painter.push_rounded_clip({bounds.x, bounds.y, bounds.w, bounds.h}, m_roundness);
    painter.fill_rect(ind_anim_rect, m_color_fill);
    painter.pop_clip();
}

void ProgressBar::draw_content(Painter& painter) {
    switch (m_type) {
    case Izo::ProgressBar::Type::Normal:
        draw_normal(painter);
        break;
    case Izo::ProgressBar::Type::Indeterminate:
        draw_indeterminate(painter);
        break;
    }
}

void ProgressBar::update() {
    switch (m_type) {
    case Izo::ProgressBar::Type::Indeterminate:
        m_indeterminate_anim.update(Application::the().delta());
    default:
        break;
    }
}

void ProgressBar::measure(int parent_w, int parent_h) {
    m_measured_size = {0, 0, 100, height()};
}

bool ProgressBar::on_touch_event(IntPoint point, bool down) {
     return false; 
}

} 
