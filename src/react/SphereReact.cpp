#include <SphereReact.hpp>

namespace SphereUI {

FlexNode::Ptr RenderReactApp(JavaScriptEngine& engine,
                             const std::string& entryScript,
                             const std::string& filename) {
    engine.installSphereUIGlobal();
    if (!engine.eval(entryScript, filename)) {
        return nullptr;
    }
    return engine.takePendingRoot();
}

} // namespace SphereUI

