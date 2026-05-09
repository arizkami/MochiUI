#pragma once
#include <gui/Components/TextInput.hpp>

namespace SphereUI {

class NumberInput : public TextInput {
public:
    double value = 0.0;
    double min = -1e18;
    double max = 1e18;
    double step = 1.0;
    int precision = 2;
    std::function<void(double)> onValueChanged;

    NumberInput();

    void draw(SkCanvas* canvas) override;
    bool onChar(uint32_t charCode) override;
    bool onKeyDown(uint32_t key) override;
    bool onMouseDown(float x, float y) override;
    bool onMouseWheel(float x, float y, float delta) override;

private:
    void syncTextFromValue();
    void syncValueFromText();
    SkRect getUpButtonRect() const;
    SkRect getDownButtonRect() const;
    bool isHoveringUp = false;
    bool isHoveringDown = false;
};

} // namespace SphereUI
