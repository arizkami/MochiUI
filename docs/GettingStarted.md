# Getting Started with AureliaUI

This guide will help you get started with building your first AureliaUI application.

## 1. Initialize the Application

Every AureliaUI application starts by initializing the `Application` singleton.

```cpp
#include <AUKApplication.hpp>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    AureliaUI::Application::getInstance().init();
    // ...
}
```

## 2. Create a Window

AureliaUI provides a `Win32Window` class for creating windows on Windows.

```cpp
#include <AUKFoundation.hpp>

AureliaUI::Win32Window window("My AureliaUI App", 800, 600);
window.enableMica(true); // Optional: Enable Windows 11 Mica effect
```

## 3. Build the UI Tree

AureliaUI uses a tree of `FlexNode` objects for the UI. You can use standard nodes or create custom ones.

```cpp
using namespace AureliaUI;

auto root = FlexNode::Column();
root->style.backgroundColor = Theme::Background;
root->style.setWidthFull();
root->style.setHeightFull();
root->style.setPadding(20);

auto title = std::make_shared<TextNode>("Hello, AureliaUI!");
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
#include <AUKApplication.hpp>
#include <AUKGraphicComponents.hpp>

using namespace AureliaUI;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Application::getInstance().init();

    Win32Window window("Hello World", 400, 300);

    auto root = FlexNode::Column();
    root->style.backgroundColor = Theme::Background;
    root->style.setJustifyContent(YGJustifyCenter);
    root->style.setAlignItems(YGAlignCenter);
    root->style.setWidthFull();
    root->style.setHeightFull();

    auto text = std::make_shared<TextNode>("Welcome to AureliaUI");
    text->color = Theme::TextPrimary;
    root->addChild(text);

    window.setRoot(root);
    window.run();

    return 0;
}
```
