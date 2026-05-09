#include <SphereVUE.hpp>

namespace SphereUI {

FlexNode::Ptr RenderVueApp(JavaScriptEngine& engine,
                           const std::string& entryScript,
                           const std::string& filename) {
    engine.installSphereVueGlobal();
    if (!engine.eval(entryScript, filename)) {
        return nullptr;
    }
    return engine.takePendingRoot();
}

} // namespace SphereUI
