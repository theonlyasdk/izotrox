#pragma once

#include "Motion/Animator.hpp"
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
    void set_type(ProgressBar::Type type);
    void set_animation_variant(AnimationVariant variant);

    ProgressBar::Type type() const;
    float progress() const;
    AnimationVariant animation_variant() const;

    void draw_content(Painter& painter) override;
    void measure(int parent_w, int parent_h) override;
    bool on_touch_event(IntPoint point, bool down) override;
    void update() override;

private:
    float m_value;
    AnimationVariant m_variant{AnimationVariant::Variant1};
    ProgressBar::Type m_type;
    Animator<float> m_indeterminate_anim{0.0f};
    void draw_normal(Painter& painter);
    void draw_indeterminate(Painter& painter);
};

} 
