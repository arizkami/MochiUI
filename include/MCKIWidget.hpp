#pragma once
#include <MCKGraphicInterface.hpp>

namespace MochiUI {

// High-level window widget — prefer this over Win32Window directly
class MochiWidget : public Win32Window {
public:
    MochiWidget(const std::string& title, int width, int height);
    virtual ~MochiWidget() = default;

    void toggleFullscreen();

    static std::shared_ptr<MochiWidget> Create(const std::string& title, int width, int height);
};

} // namespace MochiUI
