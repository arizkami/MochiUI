import json
import os
import sys


def get_sk_color(values):
    """Convert RGB/RGBA values to Skia color format"""
    if len(values) == 3:
        return f"SkColorSetRGB({values[0]}, {values[1]}, {values[2]})"
    elif len(values) == 4:
        return f"SkColorSetARGB({values[0]}, {values[1]}, {values[2]}, {values[3]})"
    return "SK_ColorTRANSPARENT"


def get_available_themes():
    """Get list of available theme files"""
    themes_dir = "themes"
    if not os.path.exists(themes_dir):
        return []

    themes = []
    for file in os.listdir(themes_dir):
        if file.endswith(".json"):
            themes.append(file[:-5])  # Remove .json extension
    return themes


def load_theme_data(theme_name):
    """Load theme data from JSON file"""
    theme_file = os.path.join("themes", f"{theme_name}.json")

    if not os.path.exists(theme_file):
        print(f"Error: Theme '{theme_name}.json' not found!")
        return None

    try:
        with open(theme_file, "r") as f:
            data = json.load(f)

        # Validate required theme properties
        required_keys = [
            "background",
            "sidebar",
            "menubar",
            "accent",
            "text_primary",
            "text_secondary",
            "hover_overlay",
            "card",
        ]

        for key in required_keys:
            if key not in data:
                print(f"Error: Missing required theme property '{key}' in {theme_file}")
                return None

        return data
    except Exception as e:
        print(f"Error loading theme {theme_name}: {e}")
        return None


def generate_theme():
    """Generate Theme.hpp with all available themes"""
    themes = get_available_themes()
    print(f"Generating theme header for: {', '.join(themes)}")

    theme_data_map = {}
    for theme in themes:
        data = load_theme_data(theme)
        if data:
            theme_data_map[theme] = data

    if not theme_data_map:
        print("Error: No valid themes found in themes/ directory")
        return False

    # Ensure dark and light are present as defaults if not found
    if "dark" not in theme_data_map:
        print(
            "Warning: 'dark' theme not found, using first available theme as default dark"
        )
        theme_data_map["dark"] = list(theme_data_map.values())[0]
    if "light" not in theme_data_map:
        print("Warning: 'light' theme not found, using 'dark' as default light")
        theme_data_map["light"] = theme_data_map["dark"]

    try:
        header_lines = [
            "#pragma once",
            "#include <include/core/SkColor.h>",
            "",
            "namespace AureliaUI {",
            "",
        ]

        # Generate namespaces for each theme
        for theme, data in theme_data_map.items():
            # Use specific casing for known themes to match ThemeSwitcher.cpp
            if theme == "system":
                namespace_name = "SystemTheme"
            elif theme == "minimal":
                namespace_name = "MinimalTheme"
            else:
                namespace_name = theme.capitalize() + "Theme"

            header_lines.append(f"// {theme.capitalize()} Theme")
            header_lines.append(f"namespace {namespace_name} {{")
            header_lines.append(
                f"static constexpr SkColor Background    = {get_sk_color(data['background'])};"
            )
            header_lines.append(
                f"static constexpr SkColor Sidebar       = {get_sk_color(data['sidebar'])};"
            )
            header_lines.append(
                f"static constexpr SkColor MenuBar       = {get_sk_color(data['menubar'])};"
            )
            header_lines.append(
                f"static constexpr SkColor Accent        = {get_sk_color(data['accent'])};"
            )
            header_lines.append(
                f"static constexpr SkColor TextPrimary   = {get_sk_color(data['text_primary'])};"
            )
            header_lines.append(
                f"static constexpr SkColor TextSecondary = {get_sk_color(data['text_secondary'])};"
            )
            header_lines.append(
                f"static constexpr SkColor HoverOverlay  = {get_sk_color(data['hover_overlay'])};"
            )
            header_lines.append(
                f"static constexpr SkColor Card          = {get_sk_color(data['card'])};"
            )
            header_lines.append(
                f"static constexpr SkColor Border        = {get_sk_color(data.get('border', [50, 50, 50]))};"
            )
            header_lines.append(
                f"static constexpr SkColor Shadow        = {get_sk_color(data.get('shadow', [0, 0, 0, 100]))};"
            )
            header_lines.append(f"}} // namespace {namespace_name}")
            header_lines.append("")

        # Active Theme - defaults to Dark, can be switched at runtime
        header_lines.append(
            "// Active Theme - defaults to Dark, can be switched at runtime"
        )
        header_lines.append("namespace Theme {")
        header_lines.append("inline SkColor Background    = DarkTheme::Background;")
        header_lines.append("inline SkColor Sidebar       = DarkTheme::Sidebar;")
        header_lines.append("inline SkColor MenuBar       = DarkTheme::MenuBar;")
        header_lines.append("inline SkColor Accent        = DarkTheme::Accent;")
        header_lines.append("inline SkColor TextPrimary   = DarkTheme::TextPrimary;")
        header_lines.append("inline SkColor TextSecondary = DarkTheme::TextSecondary;")
        header_lines.append("inline SkColor HoverOverlay  = DarkTheme::HoverOverlay;")
        header_lines.append("inline SkColor Card          = DarkTheme::Card;")
        header_lines.append("inline SkColor Border        = DarkTheme::Border;")
        header_lines.append("inline SkColor Shadow        = DarkTheme::Shadow;")
        header_lines.append("")
        header_lines.append("// Non-color theme properties")
        header_lines.append("inline float BorderWidth     = 1.0f;")
        header_lines.append("inline float BorderRadius    = 4.0f;")
        header_lines.append("inline float ControlHeight   = 32.0f;")
        header_lines.append("inline float FontSmall       = 12.0f;")
        header_lines.append("inline float FontNormal      = 14.0f;")
        header_lines.append("inline float FontMedium      = 16.0f;")
        header_lines.append("inline float FontLarge       = 18.0f;")
        header_lines.append("inline float FontHeader      = 24.0f;")
        header_lines.append("} // namespace Theme")
        header_lines.append("")
        header_lines.append("} // namespace AureliaUI")

        theme_content = "\n".join(header_lines)

        # Write to include directory
        output_path = os.path.join("include", "gui", "Theme.hpp")
        os.makedirs(os.path.dirname(output_path), exist_ok=True)

        with open(output_path, "w") as f:
            f.write(theme_content)

        print(f"Theme header generated successfully at {output_path}")

        # Also write to old location for backward compatibility
        old_output_path = os.path.join("src", "gui", "Theme.hpp")
        os.makedirs(os.path.dirname(old_output_path), exist_ok=True)

        with open(old_output_path, "w") as f:
            f.write(theme_content)

        return True

    except Exception as e:
        print(f"Error generating theme: {e}")
        return False


def main():
    """Main entry point"""
    # Generate theme header
    success = generate_theme()

    if not success:
        sys.exit(1)


if __name__ == "__main__":
    main()
