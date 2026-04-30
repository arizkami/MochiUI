#include <gui/Components/Table.hpp>
#include <gui/Components/TextNode.hpp>

namespace MochiUI {

TableHead::TableHead() {
    style.setFlexDirection(YGFlexDirectionRow);
    style.setHeight(32.0f);
    style.setAlignItems(YGAlignCenter);
    style.backgroundColor = SkColorSetA(Theme::TextSecondary, 30);
}

void TableHead::addColumn(const std::string& title, float width) {
    auto text = std::make_shared<TextNode>();
    text->text = title;
    text->fontSize = 12.0f;
    text->color = Theme::TextSecondary;
    text->fontFamily = fontFamily;
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
    row->enableHover = true;

    // Zebra striping
    int rowCount = (int)children.size();
    if (header) rowCount--; // Don't count header for parity
    
    if (rowCount % 2 == 1) {
        row->style.backgroundColor = SkColorSetA(Theme::TextSecondary, 15);
    }
    
    // Add hover highlight to row
    row->style.borderRadius = 0.0f;
    row->enableHover = true;
    if (header) {
        for (size_t i = 0; i < cells.size() && i < header->children.size(); ++i) {
            auto headCol = header->children[i];
            if (!headCol) continue;

            auto cell = std::make_shared<TextNode>();
            cell->text = cells[i];
            cell->fontSize = 13.0f;
            cell->color = Theme::TextPrimary;
            cell->fontFamily = fontFamily;
            cell->style.setPadding(8.0f);
            
            // Match width/flex of header column
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

    // Draw children but with manual row hover highlight logic
    // (Alternative: TextNode or FlexNode already handle hover if enabled)
    // Here we ensure the row itself draws its hover background if we didn't use a child component
    
    drawChildren(canvas);
    
    // Draw outer border
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SkColorSetA(Theme::TextSecondary, 40));
    canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, paint);
}

} // namespace MochiUI
