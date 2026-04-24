# MochiUI Framework

MochiUI is a modern, high-performance UI framework for Windows, built from the ground up using **Skia** for high-fidelity 2D graphics and **Yoga Layout v3.2** for a flexible, web-inspired Flexbox layout engine.

## 🚀 Key Features

-   **Modern Rendering:** Powered by Skia with Direct3D 12 backend for buttery-smooth performance and native Windows 11 Mica effects.
-   **Flexbox Layout:** Fully integrated Yoga Layout (v3.2) support with an easy-to-use C++ API for complex, responsive designs.
-   **Rich Component Library:**
    -   **Basic:** Text, Buttons, Checkboxes, Sliders, Progress Bars.
    -   **Advanced:** Data Tables, Calendars, Color Pickers.
    -   **Containers:** Scroll Areas with vertical scrolling/dragging, Group Boxes, Combo Boxes.
-   **Stable UI:** Optimized for window management with synchronous resizing and no visual jitter or flickering.
-   **Theme Support:** Built-in Dark and Light themes with easy runtime switching.
-   **Extensible:** Granular drawing pipeline (`drawSelf`/`drawChildren`) allowing for custom component implementation.

## 🏗️ Technical Architecture

MochiUI uses a **Research -> Strategy -> Execution** lifecycle for UI updates:
1.  **Yoga Layout:** Recalculates the position and size of every node based on Flexbox rules.
2.  **Skia Rendering:** Draws the UI nodes using the calculated frames, supporting advanced effects like rounded corners, shadows, and clipping.
3.  **D3D12 Backend:** Ensures low-level access to the GPU for maximum efficiency on modern Windows hardware.

## 📦 Quick Start

### Basic UI Setup

```cpp
auto root = FlexNode::Row();
root->style.setWidthFull();
root->style.setHeightFull();
root->style.backgroundColor = Theme::Background;

auto text = std::make_shared<TextNode>();
text->text = "Hello, MochiUI!";
text->fontSize = 24.0f;
text->style.setMargin(20.0f);

root->addChild(text);
window.setRoot(root);
```

### Scrolling & Layout

```cpp
auto scrollArea = std::make_shared<ScrollAreaNode>();
scrollArea->style.setFlex(1.0f);

auto longContent = FlexNode::Column();
longContent->style.setHeightAuto(); // Grow based on children
// ... add many items ...

scrollArea->setContent(longContent);
```

## 🛠️ Build Requirements

-   **CMake** 3.15+
-   **Visual Studio 2022** (with C++20 support)
-   **Windows 10/11 SDK**

The framework automatically downloads and installs the required prebuilt Skia binaries during the first CMake configure run.

## 📜 License

MochiUI is licensed under the BSD License. See `LICENSE` for details.
