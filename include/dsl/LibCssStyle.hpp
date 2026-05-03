#pragma once
#include <string>

namespace AureliaUI {

class DSLNode;
struct DslInherited;

// Parse `cssText` as an inline CSS declaration block (e.g.
// `height: 64px; display: flex; align-items: center;`) using libcss, then map
// the computed values onto `node` + `inh`. `elementTag` should match the DSL
// element (`div` / `span`) for UA defaults.
//
// Returns false if libcss failed to parse or select (caller keeps prior node
// state for those properties).
bool applyLibCssDeclarationBlock(const std::string& cssText,
                                 const char* elementTag,
                                 DSLNode& node,
                                 DslInherited& inh);

} // namespace AureliaUI
