#pragma once

namespace MochiUI {

class IWindowHost {
public:
    virtual ~IWindowHost() = default;
    virtual void requestRedraw() = 0;
};

} // namespace MochiUI
