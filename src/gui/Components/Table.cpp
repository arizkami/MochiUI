#include <include/gui/Components/Table.hpp>
#include <include/gui/Components/TextNode.hpp>

namespace MochiUI {

TableHead::TableHead() {
    style.setFlexDirection(YGFlexDirectionRow);
    style.setHeight(32.0f);
    style.backgroundColor = SkColorSetA(Theme::TextSecondary, 30);
}

void TableHead::addColumn(const std::string& title, float width) {
    auto text = std::make_shared<TextNode>();
    text->text = title;
    text->fontSize = 12.0f;
    text->color = Theme::TextSecondary;
    text->style.setPadding(8.0f);
    if (width > 0) {
        text->style.setWidth(width);
    } else {
        text->style.setFlex(1.0f);
    }
    addChild(text);
}

void TableHead::draw(SkCanvas* canvas) {
    drawSelf(canvas);
    drawChildren(canvas);
    
    // Draw bottom separator
    SkPaint paint;
    paint.setColor(SkColorSetA(Theme::TextSecondary, 50));
    paint.setStrokeWidth(1.0f);
    canvas->drawLine(frame.left(), frame.bottom(), frame.right(), frame.bottom(), paint);
}

Table::Table() {
    style.setFlexDirection(YGFlexDirectionColumn);
    style.backgroundColor = Theme::Card;
    style.borderRadius = 8.0f;
    style.overflowHidden = true;
}

void Table::setHeader(std::shared_ptr<TableHead> head) {
    header = head;
    addChild(header);
}

void Table::addRow(const std::vector<std::string>& cells) {
    auto row = FlexNode::Row();
    row->style.setHeight(32.0f);
    row->style.setAlignItems(YGAlignCenter);
    
    // Simple logic: match children of header
    if (header) {
        for (size_t i = 0; i < cells.size() && i < header->children.size(); ++i) {
            auto cell = std::make_shared<TextNode>();
            cell->text = cells[i];
            cell->fontSize = 13.0f;
            cell->style.setPadding(8.0f);
            
            // Match width/flex of header column
            auto headCol = header->children[i];
            if (headCol->style.widthMode == SizingMode::Fixed) {
                cell->style.setWidth(headCol->style.width);
            } else {
                cell->style.setFlex(1.0f);
            }
            row->addChild(cell);
        }
    }
    
    addChild(row);
}

void Table::draw(SkCanvas* canvas) {
    drawSelf(canvas);
    drawChildren(canvas);
    
    // Draw outer border
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SkColorSetA(Theme::TextSecondary, 40));
    canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, paint);
}

} // namespace MochiUI
