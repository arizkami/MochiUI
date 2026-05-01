#include <gui/Components/GroupBox.hpp>

namespace AureliaUI {

GroupBox::GroupBox() {
    style.setPadding(15);
    style.setGap(10);
}

void GroupBox::syncSubtreeStyles() {
    style.syncLegacy();

    float titleHeight = 0.0f;
    if (!title.empty()) {
        SkRect bounds;
        FontManager::getInstance().measureText(title, fontSize, &bounds);
        titleHeight = bounds.height();
        // Add extra top padding to account for title
        YGNodeStyleSetPadding(getYGNode(), YGEdgeTop, style.paddingTop + titleHeight / 2.0f);
    }

    for (auto& child : children) {
        child->syncSubtreeStyles();
    }
}

void GroupBox::drawSelf(SkCanvas* canvas) {
    if (SkColorGetA(style.backgroundColor) == 0) return;

    // Background usually fills the whole frame or the border area.
    // If we want it to look like a groupbox, it should probably fill the borderRect.

    float titleHeight = 0.0f;
    if (!title.empty()) {
        SkRect bounds;
        FontManager::getInstance().measureText(title, fontSize, &bounds);
        titleHeight = bounds.height();
    }

    SkRect borderRect = frame;
    if (!title.empty()) {
        borderRect.fTop += titleHeight / 2.0f;
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(style.backgroundColor);
    float r = (style.borderRadius > 0) ? style.borderRadius : Theme::BorderRadius;
    if (r > 0) {
        canvas->drawRoundRect(borderRect, r, r, paint);
    } else {
        canvas->drawRect(borderRect, paint);
    }

    if (enableHover && isHovered) {
        SkPaint hoverPaint;
        hoverPaint.setAntiAlias(true);
        hoverPaint.setColor(SkColorSetARGB(40, 255, 255, 255));
        if (r > 0) {
            canvas->drawRoundRect(borderRect, r, r, hoverPaint);
        } else {
            canvas->drawRect(borderRect, hoverPaint);
        }
    }
}

void GroupBox::draw(SkCanvas* canvas) {
    drawSelf(canvas);

    float titleWidth = 0.0f;
    float titleHeight = 0.0f;
    SkRect titleBounds;
    SkFontMetrics metrics;

    if (!title.empty()) {
        FontManager::getInstance().measureText(title, fontSize, &titleBounds);
        titleWidth = titleBounds.width();
        titleHeight = titleBounds.height();
        FontManager::getInstance().getFontMetrics(fontSize, &metrics);
    }

    SkRect borderRect = frame;
    if (!title.empty()) {
        borderRect.fTop += titleHeight / 2.0f;
    }

    SkPaint borderPaint;
    borderPaint.setAntiAlias(true);
    borderPaint.setStyle(SkPaint::kStroke_Style);
    borderPaint.setColor(borderColor);
    borderPaint.setStrokeWidth(Theme::BorderWidth);

    if (title.empty()) {
        float r = (style.borderRadius > 0) ? style.borderRadius : Theme::BorderRadius;
        if (r > 0) {
            canvas->drawRoundRect(borderRect, r, r, borderPaint);
        } else {
            canvas->drawRect(borderRect, borderPaint);
        }
    } else {
        float titleMargin = 10.0f;
        float x1 = borderRect.left() + titleMargin;
        float x2 = x1 + titleWidth + 10.0f;

        float r = (style.borderRadius > 0) ? style.borderRadius : Theme::BorderRadius;
        if (r > 0) {
            SkPathBuilder path;
            // Top line with gap for title
            path.moveTo(x2, borderRect.top());
            path.lineTo(borderRect.right() - r, borderRect.top());
            path.arcTo(SkRect::MakeXYWH(borderRect.right() - 2*r, borderRect.top(), 2*r, 2*r), 270, 90, false);
            path.lineTo(borderRect.right(), borderRect.bottom() - r);
            path.arcTo(SkRect::MakeXYWH(borderRect.right() - 2*r, borderRect.bottom() - 2*r, 2*r, 2*r), 0, 90, false);
            path.lineTo(borderRect.left() + r, borderRect.bottom());
            path.arcTo(SkRect::MakeXYWH(borderRect.left(), borderRect.bottom() - 2*r, 2*r, 2*r), 90, 90, false);
            path.lineTo(borderRect.left(), borderRect.top() + r);
            path.arcTo(SkRect::MakeXYWH(borderRect.left(), borderRect.top(), 2*r, 2*r), 180, 90, false);
            path.lineTo(x1, borderRect.top());
            canvas->drawPath(path.detach(), borderPaint);
        } else {
            SkPathBuilder path;
            path.moveTo(x2, borderRect.top());
            path.lineTo(borderRect.right(), borderRect.top());
            path.lineTo(borderRect.right(), borderRect.bottom());
            path.lineTo(borderRect.left(), borderRect.bottom());
            path.lineTo(borderRect.left(), borderRect.top());
            path.lineTo(x1, borderRect.top());
            canvas->drawPath(path.detach(), borderPaint);
        }

        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(titleColor);

        // Center text on the line
        float textY = borderRect.top() - (metrics.fAscent + metrics.fDescent) / 2.0f;

        FontManager::getInstance().drawText(canvas, title,
                                            x1 + 5.0f, textY,
                                            fontSize, textPaint);
    }

    drawChildren(canvas);
}

} // namespace AureliaUI
