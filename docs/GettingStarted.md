# Getting Started with SphereUI

This guide will help you get started with building your first SphereUI application.

## 1. Initialize the Application

Every SphereUI application starts by initializing the `Application` singleton.

```cpp
#include <SPHXApplication.hpp>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    SphereUI::Application::getInstance().init();
    // ...
}
```

## 2. Create a Window

SphereUI provides a `Win32Window` class for creating windows on Windows.

```cpp
#include <SPHXFoundation.hpp>

SphereUI::Win32Window window("My SphereUI App", 800, 600);
window.enableMica(true); // Optional: Enable Windows 11 Mica effect
```

## 3. Build the UI Tree

SphereUI uses a tree of `FlexNode` objects for the UI. You can use standard nodes or create custom ones.

```cpp
using namespace SphereUI;

auto root = FlexNode::Column();
root->style.backgroundColor = Theme::Background;
root->style.setWidthFull();
root->style.setHeightFull();
root->style.setPadding(20);

auto title = std::make_shared<TextNode>("Hello, SphereUI!");
title->fontSize = Theme::FontLarge;
title->color    = Theme::TextPrimary;
root->addChild(title);

auto btn = std::make_shared<ButtonNode>();
btn->label = "Click Me";
btn->onClick = []() {
    // Handle click
};
root->addChild(btn);
```

## 4. Set the Root and Run

Finally, set the root node for the window and start the event loop.

```cpp
window.setRoot(root);
window.run();
```

## Complete Example

```cpp
#include <SPHXApplication.hpp>
#include <SPHXGraphicComponents.hpp>

using namespace SphereUI;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Application::getInstance().init();

    Win32Window window("Hello World", 400, 300);

    auto root = FlexNode::Column();
    root->style.backgroundColor = Theme::Background;
    root->style.setJustifyContent(YGJustifyCenter);
    root->style.setAlignItems(YGAlignCenter);
    root->style.setWidthFull();
    root->style.setHeightFull();

    auto text = std::make_shared<TextNode>("Welcome to SphereUI");
    text->color = Theme::TextPrimary;
    root->addChild(text);

    window.setRoot(root);
    window.run();

    return 0;
}
```

## Next steps

- **Native C++ UI**: build node trees directly with `FlexNode` and the built-in components.
- **Embedded assets**: pack UI text with `res://` — [Resources](Resources.md).
- **Scripted UI (Windows)**: V8 + optional React reconciler — [JavaScript and React](JavaScriptAndReact.md) and [MikoUI](MikoUI.md).
- **Build from CI or clean machine**: [Build and CI](BuildAndCI.md).
