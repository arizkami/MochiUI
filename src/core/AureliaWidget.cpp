#include <AUKIWidget.hpp>

namespace AureliaUI {

AureliaWidget::AureliaWidget(const std::string& title, int width, int height)
    : Win32Window(title, width, height) {
}

void AureliaWidget::toggleFullscreen() {
    // Basic toggle logic between Windowed and Fullscreen
    // We could check currentMode from the base class if we make it protected or provide a getter
    // Since we can't easily change the base class now, we can use a local state or just
    // try to implement it based on the public setWindowMode.

    // For now, let's assume we want to toggle between Windowed and Fullscreen
    static bool isFullscreen = false;
    if (isFullscreen) {
        setWindowMode(WindowMode::Windowed);
    } else {
        setWindowMode(WindowMode::Fullscreen);
    }
    isFullscreen = !isFullscreen;
}

std::shared_ptr<AureliaWidget> AureliaWidget::Create(const std::string& title, int width, int height) {
    return std::make_shared<AureliaWidget>(title, width, height);
}

} // namespace AureliaUI
