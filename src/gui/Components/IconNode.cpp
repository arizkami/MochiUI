#include <include/gui/Components/IconNode.hpp>
#include <include/utils/SkParsePath.h>
#include <include/core/SkPathBuilder.h>
#include <include/core/ResourceManager.hpp>
#include <fstream>
#include <sstream>
#include <regex>

namespace MochiUI {

std::string IconNode::iconsDir = "External/lucide/icons";

IconNode::IconNode() {
    style.setWidth(24);
    style.setHeight(24);
}

void IconNode::setIcon(const std::string& name) {
    if (name.empty()) return;

    if (name.size() >= 6 && name.substr(0, 6) == "res://") {
        std::string content = ResourceManager::getInstance().getResourceString(name);
        if (!content.empty()) {
            parseSVG(content);
        }
        return;
    }

    // Default to res:// if it's just a name and we have it in resources
    std::string resPath = "res://" + name + (name.find(".svg") == std::string::npos ? ".svg" : "");
    std::string content = ResourceManager::getInstance().getResourceString(resPath);
    if (!content.empty()) {
        parseSVG(content);
        return;
    }

    std::string path = iconsDir + "/" + name + (name.find(".svg") == std::string::npos ? ".svg" : "");
    std::ifstream file(path);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        parseSVG(buffer.str());
    }
}

void IconNode::parseSVG(const std::string& content) {
    commands.clear();
    
    // Simple regex parser for Lucide icons
    std::regex pathRegex("<path [^>]*d=\"([^\"]+)\"");
    std::regex circleRegex("<circle [^>]*cx=\"([^\"]+)\" [^>]*cy=\"([^\"]+)\" [^>]*r=\"([^\"]+)\"");
    std::regex rectRegex("<rect [^>]*width=\"([^\"]+)\" [^>]*height=\"([^\"]+)\" [^>]*x=\"([^\"]+)\" [^>]*y=\"([^\"]+)\"");
    std::regex lineRegex("<line [^>]*x1=\"([^\"]+)\" [^>]*y1=\"([^\"]+)\" [^>]*x2=\"([^\"]+)\" [^>]*y2=\"([^\"]+)\"");
    std::regex polylineRegex("<polyline [^>]*points=\"([^\"]+)\"");
    std::regex polygonRegex("<polygon [^>]*points=\"([^\"]+)\"");

    auto it = std::sregex_iterator(content.begin(), content.end(), pathRegex);
    auto end = std::sregex_iterator();
    for (; it != end; ++it) {
        DrawCommand cmd;
        cmd.type = DrawCommand::Path;
        SkParsePath::FromSVGString((*it)[1].str().c_str(), &cmd.path);
        commands.push_back(cmd);
    }

    it = std::sregex_iterator(content.begin(), content.end(), circleRegex);
    for (; it != end; ++it) {
        DrawCommand cmd;
        cmd.type = DrawCommand::Circle;
        cmd.center = {std::stof((*it)[1].str()), std::stof((*it)[2].str())};
        cmd.radius = std::stof((*it)[3].str());
        commands.push_back(cmd);
    }

    it = std::sregex_iterator(content.begin(), content.end(), rectRegex);
    for (; it != end; ++it) {
        DrawCommand cmd;
        cmd.type = DrawCommand::Rect;
        cmd.rect = SkRect::MakeXYWH(std::stof((*it)[3].str()), std::stof((*it)[4].str()), 
                                    std::stof((*it)[1].str()), std::stof((*it)[2].str()));
        commands.push_back(cmd);
    }

    it = std::sregex_iterator(content.begin(), content.end(), lineRegex);
    for (; it != end; ++it) {
        DrawCommand cmd;
        cmd.type = DrawCommand::Line;
        cmd.p1 = {std::stof((*it)[1].str()), std::stof((*it)[2].str())};
        cmd.p2 = {std::stof((*it)[3].str()), std::stof((*it)[4].str())};
        commands.push_back(cmd);
    }

    auto parsePoints = [](const std::string& pts, bool close) {
        SkPathBuilder builder;
        std::stringstream ss(pts);
        float x, y;
        bool first = true;
        while (ss >> x) {
            if (ss.peek() == ',' || ss.peek() == ' ') ss.ignore();
            if (!(ss >> y)) break;
            if (first) {
                builder.moveTo(x, y);
                first = false;
            } else {
                builder.lineTo(x, y);
            }
            if (ss.peek() == ',' || ss.peek() == ' ') ss.ignore();
        }
        if (close) builder.close();
        return builder.detach();
    };

    it = std::sregex_iterator(content.begin(), content.end(), polylineRegex);
    for (; it != end; ++it) {
        DrawCommand cmd;
        cmd.type = DrawCommand::Polyline;
        cmd.path = parsePoints((*it)[1].str(), false);
        commands.push_back(cmd);
    }

    it = std::sregex_iterator(content.begin(), content.end(), polygonRegex);
    for (; it != end; ++it) {
        DrawCommand cmd;
        cmd.type = DrawCommand::Polyline; // reuse polyline logic but close it
        cmd.path = parsePoints((*it)[1].str(), true);
        commands.push_back(cmd);
    }
}

void IconNode::draw(SkCanvas* canvas) {
    drawSelf(canvas);
    
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(strokeWidth);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeJoin(SkPaint::kRound_Join);

    canvas->save();
    // Scale to fit frame
    float scaleX = frame.width() / viewBox.width();
    float scaleY = frame.height() / viewBox.height();
    canvas->translate(frame.left(), frame.top());
    canvas->scale(scaleX, scaleY);

    for (const auto& cmd : commands) {
        switch (cmd.type) {
            case DrawCommand::Path:
            case DrawCommand::Polyline:
                canvas->drawPath(cmd.path, paint);
                break;
            case DrawCommand::Circle:
                canvas->drawCircle(cmd.center, cmd.radius, paint);
                break;
            case DrawCommand::Rect:
                canvas->drawRect(cmd.rect, paint);
                break;
            case DrawCommand::Line:
                canvas->drawLine(cmd.p1, cmd.p2, paint);
                break;
        }
    }
    canvas->restore();
    
    drawChildren(canvas);
}

} // namespace MochiUI
