# Core API Reference

## Application

The `Application` class is a singleton that manages the global state of the UI framework.

### Methods

- `static Application& getInstance()`: Returns the singleton instance.
- `void init()`: Initializes the framework.

---

## IWindow

Interface for window implementations.

### Methods

- `virtual void setTitle(const std::string& title)`: Sets the window title.
- `virtual void setSize(int width, int height)`: Sets the window size.
- `virtual void setRoot(FlexNode::Ptr node)`: Sets the root UI node.
- `virtual void run()`: Starts the window event loop.
- `virtual void enableMica(bool enable)`: Enables/disables the Mica effect (Windows 11).
- `virtual void setDarkMode(bool enable)`: Sets the window to dark or light mode.

---

## FlexNode

The base class for all UI elements. It uses Yoga for layout and provides basic event handling.

### Properties

- `LayoutStyle style`: Layout configuration for the node.
- `SkRect frame`: The calculated frame of the node (after layout).
- `std::vector<Ptr> children`: List of child nodes.
- `std::function<void()> onClick`: Callback for click events.

### Methods

- `static Ptr Create()`: Creates a new FlexNode.
- `static Ptr Row()`: Creates a node with row layout direction.
- `static Ptr Column()`: Creates a node with column layout direction.
- `void addChild(Ptr child)`: Adds a child node.
- `void removeChild(Ptr child)`: Removes a child node.
- `virtual void draw(SkCanvas* canvas)`: Draws the node and its children.
- `virtual void onMouseEnter()`: Called when the mouse enters the node.
- `virtual void onMouseLeave()`: Called when the mouse leaves the node.

---

## LayoutStyle

Configuration for Yoga layout.

### Methods

- `void setWidth(float w)` / `void setHeight(float h)`: Sets fixed size.
- `void setWidthFull()` / `void setHeightFull()`: Sets size to 100%.
- `void setPadding(float p)` / `void setMargin(float m)`: Sets padding/margin.
- `void setFlex(float f)`: Sets flex grow factor.
- `void setAlignItems(YGAlign align)`: Sets alignment of children.
- `void setJustifyContent(YGJustify justify)`: Sets justification of children.
