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
        SkRect bounds;
        FontManager::getInstance().measureText(title, fontSize, &bounds);
        size.height += bounds.height() / 2.0f; // Add space for title overlapping the border
    }
    
    return size;
}

void GroupBox::calculateLayout(SkRect availableSpace) {
    FlexNode::calculateLayout(availableSpace);
    
    if (!title.empty() && !children.empty()) {
        SkRect bounds;
        FontManager::getInstance().measureText(title, fontSize, &bounds);
        float yOffset = bounds.height() / 2.0f;
        
        // Shift all children down by the title offset
        for (auto& child : children) {
            child->frame.offset(0, yOffset);
            
            // Recursively update layout for shifted children to ensure hit testing aligns
            // Actually, offset is enough since size didn't change, but hitTest uses frame.
            // Let's recursively offset their children too.
            std::function<void(FlexNode::Ptr, float)> recursiveOffset = [&](FlexNode::Ptr node, float dy) {
                for (auto& n : node->children) {
                    n->frame.offset(0, dy);
                    recursiveOffset(n, dy);
                }
            };
            recursiveOffset(child, yOffset);
        }
    }
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

    if (!title.empty()) {
        FontManager::getInstance().measureText(title, fontSize, &titleBounds);
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
        FontManager::getInstance().drawText(canvas, title, 
                                            x1 + 5.0f, borderRect.top() - titleBounds.top() - titleHeight / 2.0f, 
                                            fontSize, textPaint);
    }

    // 3. Draw Children
    for (auto& child : children) {
        child->draw(canvas);
    }
}

} // namespace MochiUI
