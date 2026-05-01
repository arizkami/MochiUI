#pragma once
#include <memory>

namespace AureliaUI {
class FlexNode;

class IWindowHost {
public:
    virtual ~IWindowHost() = default;
    virtual void requestRedraw() = 0;
    virtual void addOverlay(std::shared_ptr<FlexNode> node) {}
    virtual void removeOverlay(std::shared_ptr<FlexNode> node) {}
};

} // namespace AureliaUI
