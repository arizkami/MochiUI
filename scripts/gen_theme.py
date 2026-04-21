import os
import json

def get_sk_color(values):
    if len(values) == 3:
        return f"SkColorSetRGB({values[0]}, {values[1]}, {values[2]})"
    elif len(values) == 4:
        return f"SkColorSetARGB({values[0]}, {values[1]}, {values[2]}, {values[3]})"
    return "SK_ColorTRANSPARENT"

# Default theme
selected_theme = "dark"

# Try to read theme_config.json
config_path = "theme_config.json"
if os.path.exists(config_path):
    try:
        with open(config_path, "r") as f:
            config = json.load(f)
            selected_theme = config.get("theme", "dark")
    except:
        pass

theme_file = os.path.join("themes", f"{selected_theme}.json")
if not os.path.exists(theme_file):
    theme_file = os.path.join("themes", "dark.json")

print(f"Generating theme from: {theme_file}")

with open(theme_file, "r") as f:
    data = json.load(f)

theme_content = f"""#pragma once
#include <include/core/SkColor.h>

namespace MochiUI {{
namespace Theme {{

// Generated Theme Colors from {selected_theme}.json
static constexpr SkColor Background    = {get_sk_color(data['background'])};
static constexpr SkColor Sidebar       = {get_sk_color(data['sidebar'])};
static constexpr SkColor MenuBar       = {get_sk_color(data['menubar'])};
static constexpr SkColor Accent        = {get_sk_color(data['accent'])};
static constexpr SkColor TextPrimary   = {get_sk_color(data['text_primary'])};
static constexpr SkColor TextSecondary = {get_sk_color(data['text_secondary'])};
static constexpr SkColor HoverOverlay  = {get_sk_color(data['hover_overlay'])};
static constexpr SkColor Card          = {get_sk_color(data['card'])};

}} // namespace Theme
}} // namespace MochiUI
"""

output_path = os.path.join("src", "gui", "Theme.hpp")
os.makedirs(os.path.dirname(output_path), exist_ok=True)

with open(output_path, "w") as f:
    f.write(theme_content)

print(f"Theme '{selected_theme}' generated successfully at {output_path}")
