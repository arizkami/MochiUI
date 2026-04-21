#include <include/gui/Components/CheckboxNode.hpp>

namespace MochiUI {

Size CheckboxNode::measure(Size available) {
    float labelWidth = 0;
    float labelHeight = fontSize;
    
    if (!label.empty()) {
        SkFont font = FontManager::getInstance().createFont(
            FontManager::DEFAULT_FONT, fontSize);
        SkRect bounds;
        font.measureText(label.c_str(), label.size(), SkTextEncoding::kUTF8, &bounds);
        labelWidth = bounds.width();
        labelHeight = bounds.height();
    }
    
    float totalWidth = checkboxSize + spacing + labelWidth + 2 * style.padding;
    float totalHeight = std::max(checkboxSize, labelHeight) + 2 * style.padding;
    
    return { totalWidth, totalHeight };
}

void CheckboxNode::draw(SkCanvas* canvas) {
    if (!canvas) return;
    
    FlexNode::draw(canvas);
    
    float checkboxX = frame.left() + style.padding;
    float checkboxY = frame.centerY() - checkboxSize / 2;
    
    SkRect checkboxRect = SkRect::MakeXYWH(checkboxX, checkboxY, checkboxSize, checkboxSize);
    
    SkPaint boxPaint;
    boxPaint.setAntiAlias(true);
    boxPaint.setStyle(SkPaint::kStroke_Style);
    boxPaint.setStrokeWidth(2.0f);
    boxPaint.setColor(isHovered ? checkboxColor : SkColorSetA(checkboxColor, 180));
    
    float radius = 4.0f;
    canvas->drawRoundRect(checkboxRect, radius, radius, boxPaint);
    
    if (checked) {
        SkPaint fillPaint;
        fillPaint.setAntiAlias(true);
        fillPaint.setColor(checkboxColor);
        
        SkRect innerRect = checkboxRect;
        innerRect.inset(3, 3);
        canvas->drawRoundRect(innerRect, radius - 1, radius - 1, fillPaint);
        
        SkPaint checkPaint;
        checkPaint.setAntiAlias(true);
        checkPaint.setStyle(SkPaint::kStroke_Style);
        checkPaint.setStrokeWidth(2.0f);
        checkPaint.setColor(SK_ColorWHITE);
        checkPaint.setStrokeCap(SkPaint::kRound_Cap);
        
        float cx = checkboxRect.centerX();
        float cy = checkboxRect.centerY();
        float size = checkboxSize * 0.3f;
        
        // Draw checkmark using lines
        canvas->drawLine(cx - size * 0.5f, cy, 
                        cx - size * 0.1f, cy + size * 0.5f, checkPaint);
        canvas->drawLine(cx - size * 0.1f, cy + size * 0.5f, 
                        cx + size * 0.6f, cy - size * 0.4f, checkPaint);
    }
    
    if (!label.empty()) {
        SkFont font = FontManager::getInstance().createFont(
            FontManager::DEFAULT_FONT, fontSize);
        
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(labelColor);
        
        SkFontMetrics metrics;
        font.getMetrics(&metrics);
        
        float textX = checkboxX + checkboxSize + spacing;
        float textY = frame.centerY() - (metrics.fAscent + metrics.fDescent) / 2;
        
        canvas->drawSimpleText(label.c_str(), label.size(), SkTextEncoding::kUTF8,
                              textX, textY, font, textPaint);
    }
}

bool CheckboxNode::onMouseDown(float x, float y) {
    if (hitTest(x, y)) {
        checked = !checked;
        if (onChanged) {
            onChanged(checked);
        }
        return true;
    }
    return FlexNode::onMouseDown(x, y);
}

} // namespace MochiUI
