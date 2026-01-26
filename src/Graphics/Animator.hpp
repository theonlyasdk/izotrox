#pragma once

#include <cmath>
#include "Color.hpp"

namespace Izo {

enum class Easing {
    Linear,

    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,

    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,

    EaseInQuart,
    EaseOutQuart,
    EaseInOutQuart,

    EaseInQuint,
    EaseOutQuint,
    EaseInOutQuint,

    BounceIn,
    BounceOut,
    BounceInOut,

    SpringEaseIn,
    SpringEaseOut,
    SpringEaseBounce
};

template <typename T>
class Animator {
public:
    Animator(T startVal);
    Animator();

    void set_target(T targetVal, float durationMs, Easing ease = Easing::Linear);

    void snap_to(T targetVal);
    T value() const;
    bool is_running() const;

    void set_loop(bool loop);

    bool update(float dtMs);

private:
    float apply_easing(float t) const;
    T interpolate(const T& a, const T& b, float t);
    T current;
    T start;
    T target;
    float duration = 0.0f;
    float elapsed = 0.0f;
    Easing easing = Easing::EaseInOutQuad;
    bool running = false;
    bool m_loop = false;
};

} 