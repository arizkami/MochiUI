#pragma once
#include <SPHXGraphicInterface.hpp>
#include <core/events/Events.hpp>

namespace SphereUI {

// High-level window widget — prefer this over Win32Window directly
class SphereWidget : public Win32Window {
public:
    SphereWidget(const std::string& title, int width, int height);
    virtual ~SphereWidget() = default;

    void toggleFullscreen();

    static std::shared_ptr<SphereWidget> Create(const std::string& title, int width, int height);
};

} // namespace SphereUI
