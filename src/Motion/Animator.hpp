#pragma once

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
    SpringEaseBounce,
};

enum class AnimationLoopMode {
    NoLoop,
    ReverseOnLoop,
};

template <typename T>
class Animator {
public:
    Animator(T startVal);
    Animator();

    void set_target(T targetVal, float durationMs, Easing ease = Easing::Linear);
    void snap_to(T targetVal);
    void set_loop(bool loop);
    void set_loop_mode(AnimationLoopMode mode);
    bool update(float dtMs);

    AnimationLoopMode loop_mode() const;
    T value() const;
    bool running() const;
    float duration() const;
    int loop_count() const;

private:
    float apply_easing(float t) const;
    T interpolate(const T& a, const T& b, float t);
    T current;
    T start;
    T target;
    float m_duration = 0.0f;
    float elapsed = 0.0f;
    int m_loop_count = 0;
    Easing easing = Easing::EaseInOutQuad;
    AnimationLoopMode m_loop_mode = AnimationLoopMode::ReverseOnLoop;
    bool m_running = false;
    bool m_loop = false;
    bool m_reversing = false;
};

} 