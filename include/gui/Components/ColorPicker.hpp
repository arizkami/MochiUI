#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <functional>

namespace AureliaUI {

class ColorPicker : public FlexNode {
public:
    ColorPicker();
    SkColor getColor() const { return currentColor; }
    void setColor(SkColor color);

    std::function<void(SkColor)> onColorChanged;

    void draw(SkCanvas* canvas) override;
    Size measure(Size available) override;

    bool onMouseDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;
    void onMouseUp(float x, float y) override;

private:
    SkColor currentColor = SK_ColorRED;
    float h = 0, s = 1, v = 1;
    bool draggingSV = false;
    bool draggingH = false;

    void updateFromHSV();
    void updateFromXY(float x, float y);
};

} // namespace AureliaUI
