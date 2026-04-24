#include <include/gui/Components/GroupBox.hpp>
#include <include/core/SkPathBuilder.h>

namespace MochiUI {

GroupBox::GroupBox() {
    style.setPadding(15);
    style.setGap(10);
}

Size GroupBox::measure(Size available) {
    return { 0, 0 }; // Yoga handles children, GroupBox title is handled by extra padding in calculateLayout
}

void GroupBox::calculateLayout(SkRect availableSpace) {
    syncSubtreeStyles(); // Sync standard styles first
    
    float titleHeight = 0.0f;
    if (!title.empty()) {
        SkRect bounds;
        FontManager::getInstance().measureText(title, fontSize, &bounds);
        titleHeight = bounds.height();
        // Add extra top padding to account for title, AFTER syncLegacy
        YGNodeStyleSetPadding(getYGNode(), YGEdgeTop, style.padding + titleHeight / 2.0f);
    }
    
    YGNodeCalculateLayout(getYGNode(), availableSpace.width(), availableSpace.height(), YGDirectionLTR);
    applyYogaLayout(availableSpace.left(), availableSpace.top());
}

void GroupBox::draw(SkCanvas* canvas) {
    drawSelf(canvas);

    SkPaint borderPaint;
    borderPaint.setAntiAlias(true);
    borderPaint.setStyle(SkPaint::kStroke_Style);
    borderPaint.setColor(borderColor);
    borderPaint.setStrokeWidth(strokeWidth);

    float titleMargin = 10.0f;
    float titleWidth = 0.0f;
    float titleHeight = 0.0f;
    SkRect titleBounds;

    if (!title.empty()) {
        FontManager::getInstance().measureText(title, fontSize, &titleBounds);
        titleWidth = titleBounds.width();
        titleHeight = titleBounds.height();
    }

    SkRect borderRect = frame;
    if (!title.empty()) {
        borderRect.fTop += titleHeight / 2.0f;
    }

    if (title.empty()) {
        canvas->drawRect(borderRect, borderPaint);
    } else {
        SkPathBuilder path;
        float x1 = borderRect.left() + titleMargin;
        float x2 = x1 + titleWidth + 10.0f;
        
        path.moveTo(x2, borderRect.top());
        path.lineTo(borderRect.right(), borderRect.top());
        path.lineTo(borderRect.right(), borderRect.bottom());
        path.lineTo(borderRect.left(), borderRect.bottom());
        path.lineTo(borderRect.left(), borderRect.top());
        path.lineTo(x1, borderRect.top());
        
        canvas->drawPath(path.detach(), borderPaint);
        
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(titleColor);
        FontManager::getInstance().drawText(canvas, title, 
                                            x1 + 5.0f, borderRect.top() + titleHeight / 2.0f - 4.0f, 
                                            fontSize, textPaint);
    }

    drawChildren(canvas);
}

} // namespace MochiUI
