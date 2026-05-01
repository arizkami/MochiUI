#pragma once
#include <chrono>
#include <functional>
#include <vector>
#include <algorithm>

namespace AureliaUI {

struct Animation {
    float startValue;
    float endValue;
    std::chrono::milliseconds duration;
    std::chrono::steady_clock::time_point startTime;
    std::function<void(float)> onUpdate;
    std::function<void()> onComplete;
    bool finished = false;
};

class AnimationController {
public:
    static AnimationController& getInstance() {
        static AnimationController instance;
        return instance;
    }

    void addAnimation(Animation anim) {
        anim.startTime = std::chrono::steady_clock::now();
        animations.push_back(anim);
    }

    void update() {
        auto now = std::chrono::steady_clock::now();
        animations.erase(std::remove_if(animations.begin(), animations.end(), [&](Animation& anim) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - anim.startTime);
            float progress = std::clamp((float)elapsed.count() / anim.duration.count(), 0.0f, 1.0f);

            float currentValue = anim.startValue + (anim.endValue - anim.startValue) * progress;
            anim.onUpdate(currentValue);

            if (progress >= 1.0f) {
                if (anim.onComplete) anim.onComplete();
                return true;
            }
            return false;
        }), animations.end());
    }

    bool hasAnimations() const { return !animations.empty(); }

private:
    AnimationController() = default;
    std::vector<Animation> animations;
};

} // namespace AureliaUI
