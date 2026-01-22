#pragma once
#include "Graphics/Animator.hpp"
#include "Color.hpp"
#include <cmath>

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

template <typename T> float Animator<T>::apply_easing(float t) {
  switch (easing) {
  case Easing::EaseInQuad:
    return t * t;
  case Easing::EaseOutQuad:
    return t * (2 - t);
  case Easing::EaseInOutQuad:
    return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
  default:
    return t;
  }
}

template <>
inline float Animator<float>::interpolate(float const &a, float const &b,
                                          float t) {
  return a + (b - a) * t;
}

template <>
inline Color Animator<Color>::interpolate(Color const &a, Color const &b,
                                          float t) {
  return Color(
      (uint8_t)(a.r + (b.r - a.r) * t), (uint8_t)(a.g + (b.g - a.g) * t),
      (uint8_t)(a.b + (b.b - a.b) * t), (uint8_t)(a.a + (b.a - a.a) * t));
}
} // namespace Izo
