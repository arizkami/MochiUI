#pragma once
#include <AUKGraphicInterface.hpp>
#include <core/events/Events.hpp>

namespace AureliaUI {

// High-level window widget — prefer this over Win32Window directly
class AureliaWidget : public Win32Window {
public:
    AureliaWidget(const std::string& title, int width, int height);
    virtual ~AureliaWidget() = default;

    void toggleFullscreen();

    static std::shared_ptr<AureliaWidget> Create(const std::string& title, int width, int height);
};

} // namespace AureliaUI
