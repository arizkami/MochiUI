#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/core/SkPath.h>
#include <string>
#include <vector>

namespace MochiUI {

class IconNode : public FlexNode {
public:
    IconNode();
    void setIcon(const std::string& name);
    void draw(SkCanvas* canvas) override;
    
    SkColor color = Theme::TextPrimary;
    float strokeWidth = 2.0f;

    static void setIconsDirectory(const std::string& dir) { iconsDir = dir; }

private:
    struct DrawCommand {
        enum Type { Path, Circle, Rect, Line, Polyline };
        Type type;
        SkPath path;
        SkRect rect;
        SkPoint center;
        float radius;
        SkPoint p1, p2;
    };
    std::vector<DrawCommand> commands;
    SkRect viewBox = {0, 0, 24, 24};
    static std::string iconsDir;
    
    void parseSVG(const std::string& content);
};

} // namespace MochiUI
