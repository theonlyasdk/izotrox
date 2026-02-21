#pragma once

#include "Motion/Animator.hpp"
#include "Graphics/Color.hpp"
#include "UI/Widgets/Widget.hpp"

namespace Izo {

class ProgressBar : public Widget {
public:
    enum class Type {
        Normal,
        Indeterminate
    };

    /* Variant of indeterminate animation */
    enum class AnimationVariant {
        Variant1,
        Variant2,
    };

    ProgressBar(float progress = 0.0f);
    ProgressBar(bool indeterminate);

    void set_progress(float v);
    void set_type(ProgressBar::Type type) { m_type = type; }
    void set_animation_variant(AnimationVariant variant);

    ProgressBar::Type type() const { return m_type; }
    float progress() const { return m_value; }
    AnimationVariant animation_variant() const { return m_variant; }

    void draw_content(Painter& painter) override;
    void measure(int parent_w, int parent_h) override;
    bool on_touch_event(IntPoint point, bool down) override;
    void update() override;
    void on_theme_update() override;

private:
    float m_value;
    AnimationVariant m_variant{AnimationVariant::Variant1};
    ProgressBar::Type m_type = ProgressBar::Type::Normal;
    Animator<float> m_indeterminate_anim{0.0f};
    int m_roundness = 6;
    Color m_color_bg{100, 100, 100};
    Color m_color_fill{0, 255, 100};
    Color m_color_border{200, 200, 200};
    void draw_normal(Painter& painter);
    void draw_indeterminate(Painter& painter);
};

} 
