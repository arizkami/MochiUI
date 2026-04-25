import os
import json
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

def load_theme_config():
    """Load theme configuration from theme_config.json"""
    config_path = "theme_config.json"
    default_theme = "dark"
    
    if os.path.exists(config_path):
        try:
            with open(config_path, "r") as f:
                config = json.load(f)
                return config.get("theme", default_theme)
        except Exception as e:
            print(f"Warning: Failed to read theme config: {e}")
            return default_theme
    
    return default_theme

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
        required_keys = ['background', 'sidebar', 'menubar', 'accent', 
                        'text_primary', 'text_secondary', 'hover_overlay', 'card']
        
        for key in required_keys:
            if key not in data:
                print(f"Error: Missing required theme property '{key}' in {theme_file}")
                return None
        
        return data
    except Exception as e:
        print(f"Error loading theme {theme_name}: {e}")
        return None

def generate_theme():
    """Generate Theme.hpp with both light and dark themes"""
    print("Generating dual-theme header (light + dark)...")
    
    # Load both themes
    dark_data = load_theme_data("dark")
    light_data = load_theme_data("light")
    
    if not dark_data or not light_data:
        print("Error: Could not load required themes (dark.json and light.json)")
        return False
    
    try:
        # Generate theme header content with both themes
        theme_content = f"""#pragma once
#include <include/core/SkColor.h>

namespace MochiUI {{

// Dark Theme Colors
namespace DarkTheme {{
static constexpr SkColor Background    = {get_sk_color(dark_data['background'])};
static constexpr SkColor Sidebar       = {get_sk_color(dark_data['sidebar'])};
static constexpr SkColor MenuBar       = {get_sk_color(dark_data['menubar'])};
static constexpr SkColor Accent        = {get_sk_color(dark_data['accent'])};
static constexpr SkColor TextPrimary   = {get_sk_color(dark_data['text_primary'])};
static constexpr SkColor TextSecondary = {get_sk_color(dark_data['text_secondary'])};
static constexpr SkColor HoverOverlay  = {get_sk_color(dark_data['hover_overlay'])};
static constexpr SkColor Card          = {get_sk_color(dark_data['card'])};
static constexpr SkColor Border        = {get_sk_color(dark_data.get('border', [50, 50, 50]))};
static constexpr SkColor Shadow        = {get_sk_color(dark_data.get('shadow', [0, 0, 0, 100]))};
}} // namespace DarkTheme

// Light Theme Colors
namespace LightTheme {{
static constexpr SkColor Background    = {get_sk_color(light_data['background'])};
static constexpr SkColor Sidebar       = {get_sk_color(light_data['sidebar'])};
static constexpr SkColor MenuBar       = {get_sk_color(light_data['menubar'])};
static constexpr SkColor Accent        = {get_sk_color(light_data['accent'])};
static constexpr SkColor TextPrimary   = {get_sk_color(light_data['text_primary'])};
static constexpr SkColor TextSecondary = {get_sk_color(light_data['text_secondary'])};
static constexpr SkColor HoverOverlay  = {get_sk_color(light_data['hover_overlay'])};
static constexpr SkColor Card          = {get_sk_color(light_data['card'])};
static constexpr SkColor Border        = {get_sk_color(light_data.get('border', [200, 200, 200]))};
static constexpr SkColor Shadow        = {get_sk_color(light_data.get('shadow', [0, 0, 0, 40]))};
}} // namespace LightTheme

// Active Theme - defaults to Dark, can be switched at runtime
namespace Theme {{
inline SkColor Background    = DarkTheme::Background;
inline SkColor Sidebar       = DarkTheme::Sidebar;
inline SkColor MenuBar       = DarkTheme::MenuBar;
inline SkColor Accent        = DarkTheme::Accent;
inline SkColor TextPrimary   = DarkTheme::TextPrimary;
inline SkColor TextSecondary = DarkTheme::TextSecondary;
inline SkColor HoverOverlay  = DarkTheme::HoverOverlay;
inline SkColor Card          = DarkTheme::Card;
inline SkColor Border        = DarkTheme::Border;
inline SkColor Shadow        = DarkTheme::Shadow;
}} // namespace Theme

}} // namespace MochiUI
"""
        
        # Write to include directory
        output_path = os.path.join("include", "gui", "Theme.hpp")
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        
        with open(output_path, "w") as f:
            f.write(theme_content)
        
        print(f"Dual-theme header generated successfully at {output_path}")
        
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
    # List available themes
    available_themes = get_available_themes()
    if available_themes:
        print(f"Available themes: {', '.join(available_themes)}")
    
    # Generate dual-theme header
    success = generate_theme()
    
    if not success:
        sys.exit(1)

if __name__ == "__main__":
    main()
