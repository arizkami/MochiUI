#pragma once

namespace MochiUI {

class Application {
public:
    static Application& getInstance();
    void init();

private:
    Application() = default;
};

} // namespace MochiUI
