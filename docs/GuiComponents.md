# GUI Components

AureliaUI provides a rich set of built-in components.

## Common Components

### TextNode

Displays text.

- `std::string text`: The text to display.
- `float fontSize`: Font size.
- `AUKColor color`: Text color.
- `TextAlign textAlign`: Alignment (Left, Center, Right).

### ButtonNode

A clickable button.

- `std::string label`: Button text.
- `AUKColor textColor`: Color of the label.
- `AUKColor normalColor`: Background color.
- `bool useThemeColors`: Whether to use active theme colors automatically.

### CheckboxNode / SwitchNode

Toggle controls.

- `std::string label`: Associated text.
- `bool checked`: Current state.

### TextInput / NumberInput

User input fields.

- `std::string text` / `double value`: Current value.
- `std::string placeholder`: Hint text.

### ProgressBar / SliderNode

Value visualizers and controllers.

- `float value`: Normalized value (0.0 to 1.0).
- `void setIndeterminate(bool enable)`: (ProgressBar) Enables busy state.

## Advanced Components

### Table

Displays data in rows and columns.

### Calendar

A monthly calendar view.

### ScrollAreaNode

Provides a scrollable container for content.

### AccordionNode

Collapsible content sections.

### TabsNode

Managed tabbed interface.
