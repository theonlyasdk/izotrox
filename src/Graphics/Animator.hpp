#pragma once
#include <cmath>
#include "Color.hpp"

namespace Izo {

enum class Easing {
    Linear,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad
};

template <typename T>
class Animator {
public:
    Animator(T startVal) : current(startVal), start(startVal), target(startVal) {}
    Animator() : current(), start(), target() {}

    void set_target(T targetVal, float durationMs, Easing ease = Easing::Linear) {
        start = current;
        target = targetVal;
        duration = durationMs;
        elapsed = 0.0f;
        easing = ease;
        running = true;
    }

    void snap_to(T targetVal) {
        current = targetVal;
        target = targetVal;
        start = targetVal;
        running = false;
    }

    T value() const { return current; }
    bool is_running() const { return running; }

    bool update(float dtMs) {
        if (!running) return false;

        elapsed += dtMs;
        if (elapsed >= duration) {
            current = target;
            running = false;
            return true;
        }

        float t = elapsed / duration;
        float k = apply_easing(t);
        
        current = interpolate(start, target, k);
        return true;
    }

private:
    float apply_easing(float t) {
        switch (easing) {
            case Easing::EaseInQuad: return t * t;
            case Easing::EaseOutQuad: return t * (2 - t);
            case Easing::EaseInOutQuad: return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
            default: return t;
        }
    }

    T interpolate(const T& a, const T& b, float t);

    T current;
    T start;
    T target;
    float duration = 0.0f;
    float elapsed = 0.0f;
    Easing easing = Easing::Linear;
    bool running = false;
};

template<>
inline float Animator<float>::interpolate(const float& a, const float& b, float t) {
    return a + (b - a) * t;
}

template<>
inline Color Animator<Color>::interpolate(const Color& a, const Color& b, float t) {
    return Color(
        (uint8_t)(a.r + (b.r - a.r) * t),
        (uint8_t)(a.g + (b.g - a.g) * t),
        (uint8_t)(a.b + (b.b - a.b) * t),
        (uint8_t)(a.a + (b.a - a.a) * t)
    );
}

} // namespace Izo