#include <include/gui/Components/VUMeterNode.hpp>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace MochiUI {

void VUMeterNode::setValue(float val) {
    value = std::clamp(val, 0.0f, 1.0f);
    
    // Update peak if current value is higher
    if (value > peakValue) {
        peakValue = value;
        peakHoldTimer = peakHoldTime;
    }
}

void VUMeterNode::setPeak(float val) {
    peakValue = std::clamp(val, 0.0f, 1.0f);
    peakHoldTimer = peakHoldTime;
}

void VUMeterNode::update(float deltaTime) {
    // Decay peak hold
    if (peakHoldTimer > 0.0f) {
        peakHoldTimer -= deltaTime;
        if (peakHoldTimer <= 0.0f) {
            peakValue = value;
        }
    }
}

Size VUMeterNode::measure(Size available) {
    float w = vertical ? (meterWidth + (showNumber ? 40.0f : 0.0f)) : meterHeight;
    float h = vertical ? meterHeight : (meterWidth + (showNumber ? 25.0f : 0.0f));
    
    if (style.widthMode == SizingMode::Fixed) w = style.width;
    if (style.heightMode == SizingMode::Fixed) h = style.height;
    
    return { w, h };
}

void VUMeterNode::draw(SkCanvas* canvas) {
    FlexNode::draw(canvas);
    
    float meterX, meterY, meterW, meterH;
    
    if (vertical) {
        meterX = frame.left() + 5;
        meterY = frame.top() + 5;
        meterW = meterWidth;
        meterH = frame.height() - 10 - (showNumber ? 25 : 0);
    } else {
        meterX = frame.left() + 5;
        meterY = frame.top() + 5;
        meterW = frame.width() - 10 - (showNumber ? 50 : 0);
        meterH = meterWidth;
    }
    
    // Draw background
    SkPaint bgPaint;
    bgPaint.setAntiAlias(true);
    bgPaint.setColor(backgroundColor);
    SkRRect bgRect = SkRRect::MakeRectXY(
        SkRect::MakeXYWH(meterX, meterY, meterW, meterH),
        2.0f, 2.0f
    );
    canvas->drawRRect(bgRect, bgPaint);
    
    // Draw segments
    int numSegments = vertical ? 30 : 20;
    float segmentGap = 1.0f;
    
    for (int i = 0; i < numSegments; i++) {
        float segmentValue = (float)(i + 1) / numSegments;
        
        if (segmentValue > value) continue;
        
        SkPaint segmentPaint;
        segmentPaint.setAntiAlias(true);
        segmentPaint.setColor(getColorForValue(segmentValue));
        
        if (vertical) {
            float segmentHeight = (meterH / numSegments) - segmentGap;
            float segmentY = meterY + meterH - ((i + 1) * (meterH / numSegments));
            
            SkRRect segmentRect = SkRRect::MakeRectXY(
                SkRect::MakeXYWH(meterX + 2, segmentY + segmentGap, meterW - 4, segmentHeight),
                1.0f, 1.0f
            );
            canvas->drawRRect(segmentRect, segmentPaint);
        } else {
            float segmentWidth = (meterW / numSegments) - segmentGap;
            float segmentX = meterX + (i * (meterW / numSegments));
            
            SkRRect segmentRect = SkRRect::MakeRectXY(
                SkRect::MakeXYWH(segmentX + segmentGap, meterY + 2, segmentWidth, meterH - 4),
                1.0f, 1.0f
            );
            canvas->drawRRect(segmentRect, segmentPaint);
        }
    }
    
    // Draw peak indicator
    if (peakValue > 0.0f && peakHoldTimer > 0.0f) {
        SkPaint peakPaint;
        peakPaint.setAntiAlias(true);
        peakPaint.setColor(peakColor);
        
        if (vertical) {
            float peakY = meterY + meterH - (peakValue * meterH);
            canvas->drawRect(SkRect::MakeXYWH(meterX + 1, peakY - 1, meterW - 2, 2), peakPaint);
        } else {
            float peakX = meterX + (peakValue * meterW);
            canvas->drawRect(SkRect::MakeXYWH(peakX - 1, meterY + 1, 2, meterH - 2), peakPaint);
        }
    }
    
    // Draw numeric value
    if (showNumber) {
        std::string valueText = formatValue(value);
        
        SkFont font = FontManager::getInstance().createFont(
            FontManager::DEFAULT_FONT, 11.0f);
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(textColor);
        
        SkRect bounds;
        font.measureText(valueText.c_str(), valueText.size(), SkTextEncoding::kUTF8, &bounds);
        
        if (vertical) {
            float textX = meterX + meterW + 5;
            float textY = frame.bottom() - 5;
            canvas->drawSimpleText(valueText.c_str(), valueText.size(), 
                                  SkTextEncoding::kUTF8, textX, textY, font, textPaint);
        } else {
            float textX = frame.right() - bounds.width() - 5;
            float textY = meterY + meterH + 15;
            canvas->drawSimpleText(valueText.c_str(), valueText.size(), 
                                  SkTextEncoding::kUTF8, textX, textY, font, textPaint);
        }
    }
    
    // Draw scale marks
    SkPaint markPaint;
    markPaint.setAntiAlias(true);
    markPaint.setColor(SkColorSetARGB(100, 255, 255, 255));
    markPaint.setStrokeWidth(1.0f);
    
    float marks[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
    for (float mark : marks) {
        if (vertical) {
            float markY = meterY + meterH - (mark * meterH);
            canvas->drawLine(meterX - 3, markY, meterX, markY, markPaint);
        } else {
            float markX = meterX + (mark * meterW);
            canvas->drawLine(markX, meterY + meterH, markX, meterY + meterH + 3, markPaint);
        }
    }
}

float VUMeterNode::valueToDB(float normalized) const {
    if (normalized <= 0.0f) return -60.0f;
    return 20.0f * std::log10(normalized);
}

std::string VUMeterNode::formatValue(float val) const {
    std::ostringstream oss;
    
    if (useDecibels) {
        float db = valueToDB(val);
        if (db <= -60.0f) {
            oss << "-∞";
        } else {
            oss << std::fixed << std::setprecision(1) << db << "dB";
        }
    } else {
        oss << std::fixed << std::setprecision(0) << (val * 100.0f) << "%";
    }
    
    return oss.str();
}

SkColor VUMeterNode::getColorForValue(float val) const {
    if (val >= redThreshold) {
        return redColor;
    } else if (val >= yellowThreshold) {
        // Blend between green and yellow
        float t = (val - yellowThreshold) / (redThreshold - yellowThreshold);
        return SkColorSetRGB(
            SkColorGetR(greenColor) + t * (SkColorGetR(yellowColor) - SkColorGetR(greenColor)),
            SkColorGetG(greenColor) + t * (SkColorGetG(yellowColor) - SkColorGetG(greenColor)),
            SkColorGetB(greenColor) + t * (SkColorGetB(yellowColor) - SkColorGetB(greenColor))
        );
    } else {
        return greenColor;
    }
}

} // namespace MochiUI
