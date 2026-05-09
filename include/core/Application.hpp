#pragma once

namespace SphereUI {

class Application {
public:
    static Application& getInstance();
    void init();

private:
    Application() = default;
};

} // namespace SphereUI
