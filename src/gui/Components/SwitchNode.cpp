#include <gui/Components/SwitchNode.hpp>

namespace SphereUI {

SwitchNodeStyle SwitchNode::resolveStyle() const {
    SwitchNodeStyle resolved;
    resolved.activeColor = visualStyle.activeColor.value_or(activeColor);
    resolved.inactiveColor = visualStyle.inactiveColor.value_or(inactiveColor);
    resolved.thumbColor = visualStyle.thumbColor.value_or(thumbColor);
    resolved.labelColor = visualStyle.labelColor.value_or(labelColor);
    resolved.borderColor = visualStyle.borderColor.value_or(SPHXColor(Theme::Border).withAlpha(uint8_t{160}));
    resolved.shadowColor = visualStyle.shadowColor.value_or(SPHXColor(Theme::Shadow).withAlpha(uint8_t{70}));
    resolved.fontSize = visualStyle.fontSize.value_or(fontSize);
    resolved.switchWidth = visualStyle.switchWidth.value_or(switchWidth);
    resolved.switchHeight = visualStyle.switchHeight.value_or(switchHeight);
    resolved.spacing = visualStyle.spacing.value_or(spacing);
    resolved.thumbInset = visualStyle.thumbInset.value_or(3.0f);
    resolved.borderWidth = visualStyle.borderWidth.value_or(1.0f);
    return resolved;
}

Size SwitchNode::measure(Size available) {
    const auto resolved = resolveStyle();
    const float fontSize = *resolved.fontSize;
    const float switchWidth = *resolved.switchWidth;
    const float switchHeight = *resolved.switchHeight;
    const float spacing = *resolved.spacing;
    float labelWidth = 0;
    float labelHeight = fontSize;

    if (!label.empty()) {
        SkRect bounds;
        FontManager::getInstance().measureText(label, fontSize, &bounds);
        labelWidth = bounds.width();
        labelHeight = bounds.height();
    }

    float totalWidth = switchWidth + (label.empty() ? 0 : (spacing + labelWidth));
    float totalHeight = std::max(switchHeight, labelHeight);

    return { std::ceil(totalWidth), std::ceil(totalHeight) };
}

void SwitchNode::draw(SkCanvas* canvas) {
    if (!canvas) return;

    drawSelf(canvas);

    const auto resolved = resolveStyle();
    const SPHXColor activeColor = *resolved.activeColor;
    const SPHXColor inactiveColor = *resolved.inactiveColor;
    const SPHXColor thumbColor = *resolved.thumbColor;
    const SPHXColor labelColor = *resolved.labelColor;
    const SPHXColor borderColor = *resolved.borderColor;
    const SPHXColor shadowColor = *resolved.shadowColor;
    const float fontSize = *resolved.fontSize;
    const float switchWidth = *resolved.switchWidth;
    const float switchHeight = *resolved.switchHeight;
    const float spacing = *resolved.spacing;
    const float thumbInset = *resolved.thumbInset;
    const float borderWidth = *resolved.borderWidth;
    float trackX = frame.left() + getLayoutPadding(YGEdgeLeft);
    float trackY = frame.centerY() - switchHeight / 2;

    SkRect trackRect = SkRect::MakeXYWH(trackX, trackY, switchWidth, switchHeight);
    float radius = switchHeight / 2.0f;
    const float thumbSize = switchHeight - (thumbInset * 2.0f);
    const float thumbTravel = std::max(0.0f, switchWidth - thumbSize - (thumbInset * 2.0f));
    const float thumbX = trackRect.left() + thumbInset + (isOn ? thumbTravel : 0.0f);
    const float thumbY = trackRect.top() + thumbInset;
    SkRect thumbRect = SkRect::MakeXYWH(thumbX, thumbY, thumbSize, thumbSize);

    SPHXColor trackColor = isOn ? activeColor : inactiveColor;
    if (isHovered) trackColor = trackColor.lighter(0.06f);
    if (isPressed) trackColor = trackColor.darker(0.08f);

    SkPaint trackPaint;
    trackPaint.setAntiAlias(true);
    trackPaint.setColor(trackColor);

    canvas->drawRoundRect(trackRect, radius, radius, trackPaint);

    SkPaint innerPaint;
    innerPaint.setAntiAlias(true);
    innerPaint.setColor((isOn ? activeColor.lighter(0.08f) : inactiveColor.darker(0.12f)).withAlpha(uint8_t{120}));
    canvas->drawRoundRect(trackRect.makeInset(1.5f, 1.5f), radius - 1.5f, radius - 1.5f, innerPaint);

    if (borderWidth > 0.0f) {
        SkPaint borderPaint;
        borderPaint.setAntiAlias(true);
        borderPaint.setColor(borderColor);
        borderPaint.setStyle(SkPaint::kStroke_Style);
        borderPaint.setStrokeWidth(borderWidth);
        canvas->drawRoundRect(trackRect.makeInset(borderWidth * 0.5f, borderWidth * 0.5f), radius, radius, borderPaint);
    }

    SkPaint shadowPaint;
    shadowPaint.setAntiAlias(true);
    shadowPaint.setColor(shadowColor.withAlpha(isPressed ? uint8_t{50} : uint8_t{80}));
    canvas->drawRoundRect(thumbRect.makeOffset(0, isPressed ? 1.0f : 2.0f), thumbSize / 2, thumbSize / 2, shadowPaint);

    SkPaint thumbPaint;
    thumbPaint.setAntiAlias(true);
    SPHXColor th = thumbColor;
    if (isHovered) th = th.lighter(0.10f);
    if (isPressed) th = th.darker(0.05f);
    thumbPaint.setColor(th);

    canvas->drawRoundRect(thumbRect, thumbSize / 2, thumbSize / 2, thumbPaint);

    const std::string stateText = isOn ? "ON" : "OFF";
    const float stateFontSize = std::max(9.0f, fontSize - 3.0f);
    SkPaint statePaint;
    statePaint.setAntiAlias(true);
    statePaint.setColor(isOn
        ? SPHXColor::white().withAlpha(uint8_t{210})
        : labelColor.withAlpha(uint8_t{180}));
    SkRect stateBounds;
    FontManager::getInstance().measureText(stateText, stateFontSize, &stateBounds);
    const float stateTextX = isOn
        ? trackRect.left() + 10.0f
        : trackRect.right() - stateBounds.width() - 10.0f;
    const float stateTextY = trackRect.centerY() - stateBounds.centerY();
    FontManager::getInstance().drawText(canvas, stateText, stateTextX, stateTextY, stateFontSize, statePaint);

    if (!label.empty()) {
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(isHovered ? labelColor.lighter(0.06f) : labelColor);

        SkRect bounds;
        FontManager::getInstance().measureText(label, fontSize, &bounds);

        float textX = trackX + switchWidth + spacing;
        float textY = frame.centerY() - bounds.centerY();

        FontManager::getInstance().drawText(canvas, label, textX, textY, fontSize, textPaint);
    }

    drawChildren(canvas);
}

bool SwitchNode::onMouseDown(float x, float y) {
    if (hitTest(x, y)) {
        isOn = !isOn;
        if (onChanged) {
            onChanged(isOn);
        }
        return true;
    }
    return FlexNode::onMouseDown(x, y);
}

} // namespace SphereUI
