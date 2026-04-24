#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <string>
#include <vector>

namespace MochiUI {

class TableHead : public FlexNode {
public:
    TableHead();
    void addColumn(const std::string& title, float width = -1.0f); // -1 for flex
    void draw(SkCanvas* canvas) override;

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
    void draw(SkCanvas* canvas) override;

private:
    std::shared_ptr<TableHead> header;
};

} // namespace MochiUI
