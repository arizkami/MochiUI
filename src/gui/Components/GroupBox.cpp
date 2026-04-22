#include <include/gui/Components/GroupBox.hpp>
#include <include/core/SkPathBuilder.h>

namespace MochiUI {

GroupBox::GroupBox() {
    style.padding = 15;
    style.gap = 10;
}

Size GroupBox::measure(Size available) {
    Size size = FlexNode::measure(available);
    
    if (!title.empty()) {
        SkFont font = FontManager::getInstance().createFont(FontManager::DEFAULT_FONT, fontSize);
        SkRect bounds;
        font.measureText(title.c_str(), title.size(), SkTextEncoding::kUTF8, &bounds);
        size.height += bounds.height() / 2.0f; // Add space for title overlapping the border
    }
    
    return size;
}

void GroupBox::draw(SkCanvas* canvas) {
    // 1. Draw Border
    SkPaint borderPaint;
    borderPaint.setAntiAlias(true);
    borderPaint.setStyle(SkPaint::kStroke_Style);
    borderPaint.setColor(borderColor);
    borderPaint.setStrokeWidth(strokeWidth);

    float titleMargin = 10.0f;
    float titleWidth = 0.0f;
    float titleHeight = 0.0f;
    SkRect titleBounds;

    SkFont font = FontManager::getInstance().createFont(FontManager::DEFAULT_FONT, fontSize);
    if (!title.empty()) {
        font.measureText(title.c_str(), title.size(), SkTextEncoding::kUTF8, &titleBounds);
        titleWidth = titleBounds.width();
        titleHeight = titleBounds.height();
    }

    SkRect borderRect = frame;
    if (!title.empty()) {
        borderRect.fTop += titleHeight / 2.0f;
    }

    // Draw the border with a gap for the title
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
        
        // 2. Draw Title
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(titleColor);
        canvas->drawSimpleText(title.c_str(), title.size(), SkTextEncoding::kUTF8, 
                               x1 + 5.0f, borderRect.top() - titleBounds.top() - titleHeight / 2.0f, 
                               font, textPaint);
    }

    // 3. Draw Children
    // Temporarily adjust frame so children draw inside the border
    SkRect originalFrame = frame;
    frame = borderRect;
    FlexNode::draw(canvas);
    frame = originalFrame;
}

} // namespace MochiUI
