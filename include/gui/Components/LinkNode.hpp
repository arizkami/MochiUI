#pragma once
#include <gui/Components/TextNode.hpp>

namespace SphereUI {

class LinkNode : public TextNode {
public:
    LinkNode(std::string text = "") : TextNode(text) {
        color = SPHXColor::RGB(0, 120, 215); // Standard link blue
        enableHover = true;
        style.cursorType = Cursor::Hand;
        style.setPadding(0);
    }

    SPHXColor hoverColor = SPHXColor::RGB(0, 150, 255);
    bool underlineOnHover = true;

    void draw(SkCanvas* canvas) override {
        SPHXColor originalColor = color;
        if (isHovered) {
            color = hoverColor;
        }

        TextNode::draw(canvas);

        if (isHovered && underlineOnHover) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(color);
            paint.setStrokeWidth(1.0f);

            SkRect bounds;
            FontManager::getInstance().measureText(text, fontSize, &bounds);

            float x1 = frame.left() + getLayoutPadding(YGEdgeLeft);
            float x2 = x1 + bounds.width();
            float y = frame.centerY() + bounds.height() / 2 + 2.0f;

            canvas->drawLine(x1, y, x2, y, paint);
        }

        color = originalColor;
    }
};

} // namespace SphereUI
