#include <AureliaReact.hpp>

namespace AureliaUI {

FlexNode::Ptr RenderReactApp(JavaScriptEngine& engine,
                             const std::string& entryScript,
                             const std::string& filename) {
    engine.installAureliaUIGlobal();
    if (!engine.eval(entryScript, filename)) {
        return nullptr;
    }
    return engine.takePendingRoot();
}

} // namespace AureliaUI

