#pragma once

namespace AureliaUI {

class Application {
public:
    static Application& getInstance();
    void init();

private:
    Application() = default;
};

} // namespace AureliaUI
