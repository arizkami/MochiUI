#pragma once
#include <chrono>
#include <functional>
#include <vector>
#include <algorithm>
#include <cmath>

namespace SphereUI {

// ── Easing types ──────────────────────────────────────────────────────────────
enum class EasingType {
    Linear,
    EaseIn,         // cubic ease in
    EaseOut,        // cubic ease out (default)
    EaseInOut,      // cubic ease in-out
    EaseInBack,     // overshoot on enter
    EaseOutBack,    // overshoot on exit
    EaseOutBounce,  // bouncy landing
    Spring,         // damped spring oscillation
};

// ── Easing functions (t = normalized progress [0,1]) ─────────────────────────
namespace Easing {
    inline float apply(float t, EasingType type) noexcept {
        t = std::clamp(t, 0.0f, 1.0f);
        switch (type) {
        case EasingType::Linear:
            return t;
        case EasingType::EaseIn:
            return t * t * t;
        case EasingType::EaseOut: {
            float u = 1.0f - t;
            return 1.0f - u * u * u;
        }
        case EasingType::EaseInOut:
            return t < 0.5f
                ? 4.0f * t * t * t
                : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
        case EasingType::EaseInBack: {
            constexpr float c1 = 1.70158f, c3 = c1 + 1.0f;
            return c3 * t * t * t - c1 * t * t;
        }
        case EasingType::EaseOutBack: {
            constexpr float c1 = 1.70158f, c3 = c1 + 1.0f;
            float u = t - 1.0f;
            return 1.0f + c3 * u * u * u + c1 * u * u;
        }
        case EasingType::EaseOutBounce: {
            constexpr float n1 = 7.5625f, d1 = 2.75f;
            if (t < 1.0f / d1) return n1 * t * t;
            if (t < 2.0f / d1) { float s = t - 1.5f / d1;   return n1 * s * s + 0.75f;    }
            if (t < 2.5f / d1) { float s = t - 2.25f / d1;  return n1 * s * s + 0.9375f;  }
            { float s = t - 2.625f / d1; return n1 * s * s + 0.984375f; }
        }
        case EasingType::Spring: {
            constexpr float omega = 20.0f, damping = 8.0f;
            return 1.0f - std::exp(-damping * t) * std::cos(omega * t);
        }
        default: return t;
        }
    }
} // namespace Easing

// ── Animation descriptor ──────────────────────────────────────────────────────
struct Animation {
    float startValue;
    float endValue;
    std::chrono::milliseconds duration;
    std::chrono::steady_clock::time_point startTime;
    std::function<void(float)> onUpdate;
    std::function<void()> onComplete;
    EasingType easing = EasingType::EaseOut;
    bool finished = false;
};

// ── Animation controller (singleton) ─────────────────────────────────────────
class AnimationController {
public:
    static AnimationController& getInstance() {
        static AnimationController instance;
        return instance;
    }

    void addAnimation(Animation anim) {
        anim.startTime = std::chrono::steady_clock::now();
        animations.push_back(std::move(anim));
    }

    void update() {
        auto now = std::chrono::steady_clock::now();
        animations.erase(
            std::remove_if(animations.begin(), animations.end(), [&](Animation& anim) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - anim.startTime);
                float progress = std::clamp(
                    static_cast<float>(elapsed.count()) / static_cast<float>(anim.duration.count()),
                    0.0f, 1.0f);

                float easedProgress = Easing::apply(progress, anim.easing);
                float currentValue = anim.startValue + (anim.endValue - anim.startValue) * easedProgress;
                anim.onUpdate(currentValue);

                if (progress >= 1.0f) {
                    if (anim.onComplete) anim.onComplete();
                    return true;
                }
                return false;
            }),
            animations.end());
    }

    bool hasAnimations() const { return !animations.empty(); }

private:
    AnimationController() = default;
    std::vector<Animation> animations;
};

} // namespace SphereUI
