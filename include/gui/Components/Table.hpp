#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <string>
#include <vector>

namespace MochiUI {

class TableHead : public FlexNode {
public:
    TableHead();
    void addColumn(const std::string& title, float width = -1.0f); // -1 for flex
    void draw(SkCanvas* canvas) override;
    std::string fontFamily = FontManager::DEFAULT_FONT;

private:
    struct Column {
        std::string title;
        float width;
    };
    std::vector<Column> columns;
};

class Table : public FlexNode {
public:
    Table();
    void setHeader(std::shared_ptr<TableHead> head);
    void addRow(const std::vector<std::string>& cells);
    void removeRow(size_t index);
    void clearRows();
    void draw(SkCanvas* canvas) override;
    std::string fontFamily = FontManager::DEFAULT_FONT;

private:
    std::shared_ptr<TableHead> header;
};

} // namespace MochiUI
