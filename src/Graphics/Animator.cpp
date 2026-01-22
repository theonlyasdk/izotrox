#include "Graphics/Animator.hpp"
#include "Color.hpp"
#include <algorithm>

namespace Izo {
template <typename T>
Animator<T>::Animator(T startVal)
    : current(startVal), start(startVal), target(startVal) {}

template <typename T> Animator<T>::Animator() : current(), start(), target() {}

template <typename T>
void Animator<T>::set_target(T targetVal, float durationMs, Easing ease) {
  start = current;
  target = targetVal;
  duration = durationMs;
  elapsed = 0.0f;
  easing = ease;
  running = true;
}

template <typename T> void Animator<T>::snap_to(T targetVal) {
  current = targetVal;
  target = targetVal;
  start = targetVal;
  running = false;
}

template <typename T> T Animator<T>::value() const { return current; }
template <typename T> bool Animator<T>::is_running() const { return running; }

template <typename T> bool Animator<T>::update(float dtMs) {
  if (!running)
    return false;

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

// Simple bounce‑out curve (mirrors the classic “easeOutBounce”)
// – not a true physics bounce, just a convenient overshoot shape.
static float bounce_out(float p) {
    if (p < 1.0f / 2.75f) {
        return 7.5625f * p * p;
    } else if (p < 2.0f / 2.75f) {
        p -= 1.5f / 2.75f;
        return 7.5625f * p * p + 0.75f;
    } else if (p < 2.5f / 2.75f) {
        p -= 2.25f / 2.75f;
        return 7.5625f * p * p + 0.9375f;
    } else {
        p -= 2.625f / 2.75f;
        return 7.5625f * p * p + 0.984375f;
    }
}

/* Spring easing with overshoot */
static float spring(float p, bool in, bool bounce) {
    const float tension   = 0.4f;   // controls how “tight” the spring feels
    const float friction  = 0.6f;   // damping factor
    const float overshoot = bounce ? 0.1f : 0.0f;

    // Normalized time with optional ease‑in/out shaping
    float t = in ? (1.0f - std::cos(p * M_PI * 0.5f))
                 : (std::sin(p * M_PI * 0.5f));

    // Damped sinusoid – classic spring formula
    float value = 1.0f - std::exp(-t * tension) *
                        std::cos(t * (M_PI / (2.0f - friction)));

    if (bounce) {
        value += overshoot * std::sin(t * M_PI * 4.0f);
    }

    return std::clamp(value, 0.0f, 1.0f);
}

template <typename T>
float Animator<T>::apply_easing(float t) const {
    // Clamp to [0,1] for safety – most easing functions assume this range
    t = std::clamp(t, 0.0f, 1.0f);

    switch (easing) {
    case Easing::Linear:
        return t;
    case Easing::EaseInQuad:
        return t * t;
    case Easing::EaseOutQuad:
        return t * (2.0f - t);
    case Easing::EaseInOutQuad:
        return (t < 0.5f) ? (2.0f * t * t) : (-1.0f + (4.0f - 2.0f * t) * t);
    case Easing::EaseInCubic:
        return t * t * t;
    case Easing::EaseOutCubic: {
        float p = t - 1.0f;
        return p * p * p + 1.0f;
    }
    case Easing::EaseInOutCubic:
        return (t < 0.5f) ? (4.0f * t * t * t) : ((t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f);
    case Easing::EaseInQuart:
        return t * t * t * t;
    case Easing::EaseOutQuart: {
        float p = t - 1.0f;
        return 1.0f - p * p * p * p;
    }
    case Easing::EaseInOutQuart:
        return (t < 0.5f) ? (8.0f * t * t * t * t) : (1.0f - 8.0f * (t - 1.0f) * (t - 1.0f) * (t - 1.0f) * (t - 1.0f));

    case Easing::EaseInQuint:
        return t * t * t * t * t;
    case Easing::EaseOutQuint: {
        float p = t - 1.0f;
        return p * p * p * p * p + 1.0f;
    }
    case Easing::EaseInOutQuint:
        return (t < 0.5f) ? (16.0f * t * t * t * t * t) : (1.0f + 16.0f * (t - 1.0f) * (t - 1.0f) * (t - 1.0f) * (t - 1.0f) * (t - 1.0f));
    case Easing::BounceIn:
        return 1.0f - bounce_out(1.0f - t);
    case Easing::BounceOut:
        return bounce_out(t);
    case Easing::BounceInOut:
        return (t < 0.5f) ? (0.5f * (1.0f - bounce_out(1.0f - 2.0f * t))) : (0.5f * bounce_out(2.0f * t - 1.0f) + 0.5f);
    case Easing::SpringEaseIn:
        return spring(t, true, false);
    case Easing::SpringEaseOut:
        return spring(t, false, false);
    case Easing::SpringEaseBounce:
        return spring(t, false, true);

    default:
        return t;
    }
}


template <typename T>
inline T Animator<T>::interpolate(const T& a, const T &b, float t) {
  return a + (b - a) * t;
}

template <>
inline Color Animator<Color>::interpolate(Color const &a, Color const &b, float t) {
  return Color(
      (uint8_t)(a.r + (b.r - a.r) * t), (uint8_t)(a.g + (b.g - a.g) * t),
      (uint8_t)(a.b + (b.b - a.b) * t), (uint8_t)(a.a + (b.a - a.a) * t)
  );
}
} // namespace Izo

template class Izo::Animator<float>;
template class Izo::Animator<int>;
template class Izo::Animator<Izo::Color>;