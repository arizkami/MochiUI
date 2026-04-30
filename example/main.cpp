#include <MCKApplication.hpp>
#include <MCKGraphicInterface.hpp>
#include <MCKGraphicComponents.hpp>
#include <windows.h>

using namespace MochiUI;

// ── Helpers ───────────────────────────────────────────────────────────────────

static FlexNode::Ptr SectionLabel(const std::string& text) {
    auto label = std::make_shared<TextNode>(text);
    label->fontSize   = Theme::FontSmall;
    label->color      = Theme::TextSecondary;
    label->style.setPadding(0, 0, 0, 4);
    return label;
}

static FlexNode::Ptr ColorSwatch(SkColor color, const std::string& name) {
    auto col = FlexNode::Column();
    col->style.setGap(6);
    col->style.setWidth(76);
    col->style.setAlignItems(YGAlignCenter);

    auto box = std::make_shared<FlexNode>();
    box->style.setWidth(64);
    box->style.setHeight(64);
    box->style.backgroundColor = color;
    box->style.borderRadius    = Theme::BorderRadius;
    col->addChild(box);

    auto lbl = std::make_shared<TextNode>(name);
    lbl->fontSize  = Theme::FontSmall;
    lbl->color     = Theme::TextSecondary;
    lbl->textAlign = TextAlign::Center;
    col->addChild(lbl);

    return col;
}

static FlexNode::Ptr Card(FlexNode::Ptr child, const std::string& title = "") {
    auto card = FlexNode::Column();
    card->style.backgroundColor = Theme::Card;
    card->style.borderRadius    = Theme::BorderRadius;
    card->style.setPadding(16);
    card->style.setGap(12);

    if (!title.empty()) {
        auto hdr = std::make_shared<TextNode>(title);
        hdr->fontSize = Theme::FontNormal;
        hdr->color    = Theme::TextPrimary;
        card->addChild(hdr);

        auto sep = std::make_shared<SeparatorNode>();
        card->addChild(sep);
    }

    if (child) card->addChild(child);
    return card;
}

static std::shared_ptr<ScrollAreaNode> Scrollable(FlexNode::Ptr content) {
    auto scroll = std::make_shared<ScrollAreaNode>();
    scroll->setContent(content);
    return scroll;
}

// ── Tab: Controls ─────────────────────────────────────────────────────────────

static FlexNode::Ptr CreateControlsTab() {
    auto page = FlexNode::Column();
    page->style.setPadding(20);
    page->style.setGap(16);

    // ── Buttons ──────────────────────────────────────────────────────────────
    {
        auto row = FlexNode::Row();
        row->style.setGap(10);
        row->style.setFlexWrap(YGWrapWrap);

        auto mkBtn = [](const std::string& lbl, SkColor bg, SkColor fg, SkColor hover) {
            auto b = std::make_shared<ButtonNode>();
            b->label            = lbl;
            b->useThemeColors   = false;
            b->normalColor      = bg;
            b->textColor        = fg;
            b->hoverColor       = hover;
            return b;
        };

        row->addChild(mkBtn("Primary",
            Theme::Accent,
            SK_ColorWHITE,
            SkColorSetRGB(90, 155, 255)));

        row->addChild(mkBtn("Secondary",
            Theme::Card,
            Theme::TextPrimary,
            Theme::HoverOverlay));

        auto ghost = std::make_shared<ButtonNode>();
        ghost->label          = "Ghost";
        ghost->useThemeColors = false;
        ghost->normalColor    = SK_ColorTRANSPARENT;
        ghost->textColor      = Theme::Accent;
        ghost->hoverColor     = SkColorSetARGB(25, 66, 133, 244);
        row->addChild(ghost);

        row->addChild(mkBtn("Danger",
            SkColorSetRGB(196, 43, 28),
            SK_ColorWHITE,
            SkColorSetRGB(220, 60, 42)));

        page->addChild(Card(row, "Buttons"));
    }

    // ── Toggles ───────────────────────────────────────────────────────────────
    {
        auto col = FlexNode::Column();
        col->style.setGap(12);

        auto cbRow = FlexNode::Row();
        cbRow->style.setGap(24);
        cbRow->style.setAlignItems(YGAlignCenter);

        for (const auto& lbl : {"Enabled", "Disabled", "Indeterminate"}) {
            auto cb  = std::make_shared<CheckboxNode>();
            cb->label = lbl;
            cbRow->addChild(cb);
        }
        col->addChild(cbRow);

        auto swRow = FlexNode::Row();
        swRow->style.setGap(24);
        swRow->style.setAlignItems(YGAlignCenter);

        for (const auto& lbl : {"Wi-Fi", "Bluetooth", "Dark Mode"}) {
            auto sw  = std::make_shared<SwitchNode>();
            sw->label = lbl;
            swRow->addChild(sw);
        }
        col->addChild(swRow);

        page->addChild(Card(col, "Toggles"));
    }

    // ── Radio Group ───────────────────────────────────────────────────────────
    {
        auto row = FlexNode::Row();
        row->style.setGap(20);
        row->style.setAlignItems(YGAlignCenter);

        std::vector<std::shared_ptr<RadioButtonNode>> radios;
        for (const auto& lbl : {"Small", "Medium", "Large"}) {
            auto r  = std::make_shared<RadioButtonNode>();
            r->label = lbl;
            radios.push_back(r);
            row->addChild(r);
        }
        radios[1]->selected = true;

        for (size_t i = 0; i < radios.size(); ++i) {
            radios[i]->onClick = [&radios, i]() {
                for (auto& r : radios) r->selected = false;
                radios[i]->selected = true;
            };
        }

        page->addChild(Card(row, "Radio Group"));
    }

    // ── Rating ────────────────────────────────────────────────────────────────
    {
        auto col = FlexNode::Column();
        col->style.setGap(8);

        col->addChild(SectionLabel("User Rating"));
        auto rating = std::make_shared<RatingNode>(5);
        col->addChild(rating);

        page->addChild(Card(col, "Rating"));
    }

    // ── Badges ────────────────────────────────────────────────────────────────
    {
        auto row = FlexNode::Row();
        row->style.setGap(10);
        row->style.setAlignItems(YGAlignCenter);

        auto mkBadge = [](const std::string& t, SkColor bg) {
            auto b = std::make_shared<BadgeNode>(t);
            b->color = bg;
            return b;
        };

        row->addChild(mkBadge("Default",  Theme::Accent));
        row->addChild(mkBadge("Success",  SkColorSetRGB(34, 139, 34)));
        row->addChild(mkBadge("Warning",  SkColorSetRGB(218, 165, 32)));
        row->addChild(mkBadge("Danger",   SkColorSetRGB(196, 43, 28)));
        row->addChild(mkBadge("Neutral",  Theme::Card));

        page->addChild(Card(row, "Badges"));
    }

    return page;
}

// ── Tab: Inputs ───────────────────────────────────────────────────────────────

static FlexNode::Ptr CreateInputsTab() {
    auto page = FlexNode::Column();
    page->style.setPadding(20);
    page->style.setGap(16);

    // ── Text Inputs ───────────────────────────────────────────────────────────
    {
        auto col = FlexNode::Column();
        col->style.setGap(10);

        auto search = std::make_shared<SearchInputNode>();
        col->addChild(search);

        auto text = std::make_shared<TextInput>();
        text->placeholder = "Enter your name...";
        col->addChild(text);

        auto num = std::make_shared<NumberInput>();
        num->value = 42;
        col->addChild(num);

        page->addChild(Card(col, "Text & Number"));
    }

    // ── Selection ─────────────────────────────────────────────────────────────
    {
        auto col = FlexNode::Column();
        col->style.setGap(10);

        auto combo = std::make_shared<ComboBox>();
        combo->items         = {"Dark", "Light", "MD3 Dark", "MD3 Light", "Minimal", "WinUI Dark", "WinUI Light"};
        combo->selectedIndex = 0;
        col->addChild(combo);

        page->addChild(Card(col, "Combo Box"));
    }

    // ── Color Picker ──────────────────────────────────────────────────────────
    {
        auto cp = std::make_shared<ColorPicker>();
        cp->setColor(Theme::Accent);
        page->addChild(Card(cp, "Color Picker"));
    }

    return page;
}

// ── Tab: Visualizers ─────────────────────────────────────────────────────────

static FlexNode::Ptr CreateVisualizersTab() {
    auto page = FlexNode::Column();
    page->style.setPadding(20);
    page->style.setGap(16);

    // ── Slider + Progress ────────────────────────────────────────────────────
    {
        auto col = FlexNode::Column();
        col->style.setGap(12);

        auto prog = std::make_shared<ProgressBar>();
        prog->value = 0.45f;
        col->addChild(prog);

        auto slider = std::make_shared<SliderNode>();
        slider->value = 0.45f;
        slider->onValueChange = [prog](float v) { prog->value = v; };
        col->addChild(slider);

        page->addChild(Card(col, "Slider → Progress"));
    }

    // ── Knobs ────────────────────────────────────────────────────────────────
    {
        auto row = FlexNode::Row();
        row->style.setGap(24);
        row->style.setJustifyContent(YGJustifyCenter);

        auto mkKnob = [](const std::string& name, float val, SkColor fill) {
            auto col = FlexNode::Column();
            col->style.setGap(6);
            col->style.setAlignItems(YGAlignCenter);

            auto k = std::make_shared<KnobNode>();
            k->value        = val;
            k->arcFillColor = fill;
            col->addChild(k);

            auto lbl = std::make_shared<TextNode>(name);
            lbl->fontSize  = Theme::FontSmall;
            lbl->color     = Theme::TextSecondary;
            lbl->textAlign = TextAlign::Center;
            col->addChild(lbl);

            return col;
        };

        row->addChild(mkKnob("Volume",  0.70f, Theme::Accent));
        row->addChild(mkKnob("Treble",  0.50f, SkColorSetRGB(72, 199, 142)));
        row->addChild(mkKnob("Bass",    0.35f, SkColorSetRGB(255, 171, 0)));
        row->addChild(mkKnob("Reverb",  0.20f, SkColorSetRGB(255, 100, 80)));

        page->addChild(Card(row, "Knobs"));
    }

    // ── Spinners ─────────────────────────────────────────────────────────────
    {
        auto row = FlexNode::Row();
        row->style.setGap(24);
        row->style.setAlignItems(YGAlignCenter);

        auto mkSpinner = [](SkColor color, float size) {
            auto s = std::make_shared<SpinnerNode>();
            s->color = color;
            s->size  = size;
            return s;
        };

        row->addChild(mkSpinner(Theme::Accent,                   20.0f));
        row->addChild(mkSpinner(SkColorSetRGB(72, 199, 142),     28.0f));
        row->addChild(mkSpinner(SkColorSetRGB(255, 171, 0),      36.0f));
        row->addChild(mkSpinner(Theme::TextSecondary,            24.0f));

        page->addChild(Card(row, "Spinners"));
    }

    // ── Accordion ────────────────────────────────────────────────────────────
    {
        auto accordion = std::make_shared<AccordionNode>();

        auto makeContent = [](const std::string& text) {
            auto t = std::make_shared<TextNode>(text);
            t->fontSize = Theme::FontNormal;
            t->color    = Theme::TextSecondary;
            return t;
        };

        accordion->addItem("What is MochiUI?",
            makeContent("MochiUI is a hardware-accelerated UI framework built on Skia and Direct3D 12 for Windows."));
        accordion->addItem("Which themes are supported?",
            makeContent("Dark, Light, MD3 Dark, MD3 Light, Minimal, WinUI Dark, WinUI Light, and System."));
        accordion->addItem("How does layout work?",
            makeContent("Layouts use the Yoga flexbox engine, mirroring CSS Flexbox semantics."));

        page->addChild(Card(accordion, "FAQ Accordion"));
    }

    return page;
}

// ── Tab: Palette ──────────────────────────────────────────────────────────────

static FlexNode::Ptr CreatePaletteTab() {
    auto page = FlexNode::Column();
    page->style.setPadding(20);
    page->style.setGap(20);

    // ── Active theme swatches ─────────────────────────────────────────────────
    {
        auto swatches = FlexNode::Row();
        swatches->style.setGap(12);
        swatches->style.setFlexWrap(YGWrapWrap);

        swatches->addChild(ColorSwatch(Theme::Background,    "Background"));
        swatches->addChild(ColorSwatch(Theme::Sidebar,       "Sidebar"));
        swatches->addChild(ColorSwatch(Theme::MenuBar,       "MenuBar"));
        swatches->addChild(ColorSwatch(Theme::Accent,        "Accent"));
        swatches->addChild(ColorSwatch(Theme::TextPrimary,   "TextPrimary"));
        swatches->addChild(ColorSwatch(Theme::TextSecondary, "TextSecondary"));
        swatches->addChild(ColorSwatch(Theme::Card,          "Card"));
        swatches->addChild(ColorSwatch(Theme::HoverOverlay,  "Hover"));
        swatches->addChild(ColorSwatch(Theme::Border,        "Border"));

        page->addChild(Card(swatches, "Active Theme"));
    }

    // ── Accent gradient row ───────────────────────────────────────────────────
    {
        auto row = FlexNode::Row();
        row->style.setGap(8);

        struct { SkColor color; const char* name; } steps[] = {
            { SkColorSetRGB(20,  80,  180), "900" },
            { SkColorSetRGB(30,  100, 210), "800" },
            { SkColorSetRGB(50,  120, 230), "700" },
            { Theme::Accent,                "600" },
            { SkColorSetRGB(90,  155, 255), "500" },
            { SkColorSetRGB(130, 180, 255), "400" },
            { SkColorSetRGB(170, 205, 255), "300" },
        };

        for (auto& s : steps)
            row->addChild(ColorSwatch(s.color, s.name));

        page->addChild(Card(row, "Accent Scale"));
    }

    // ── Semantic colors ───────────────────────────────────────────────────────
    {
        auto row = FlexNode::Row();
        row->style.setGap(12);
        row->style.setFlexWrap(YGWrapWrap);

        row->addChild(ColorSwatch(SkColorSetRGB(34,  139, 34),  "Success"));
        row->addChild(ColorSwatch(SkColorSetRGB(218, 165, 32),  "Warning"));
        row->addChild(ColorSwatch(SkColorSetRGB(196, 43,  28),  "Danger"));
        row->addChild(ColorSwatch(SkColorSetRGB(0,   165, 224), "Info"));
        row->addChild(ColorSwatch(SkColorSetRGB(208, 188, 255), "Purple"));
        row->addChild(ColorSwatch(SkColorSetRGB(72,  199, 142), "Teal"));
        row->addChild(ColorSwatch(SkColorSetRGB(255, 171, 0),   "Amber"));

        page->addChild(Card(row, "Semantic Colors"));
    }

    return page;
}

// ── Tab: Data ────────────────────────────────────────────────────────────────

static FlexNode::Ptr CreateDataTab() {
    auto page = FlexNode::Column();
    page->style.setPadding(20);
    page->style.setGap(16);

    // ── Table ─────────────────────────────────────────────────────────────────
    {
        auto table = std::make_shared<Table>();

        auto head = std::make_shared<TableHead>();
        head->addColumn("ID",     50);
        head->addColumn("Name",   160);
        head->addColumn("Status", 100);
        head->addColumn("Score",  -1);
        table->setHeader(head);

        table->addRow({"1", "Alpha",   "Active",   "98"});
        table->addRow({"2", "Beta",    "Inactive", "74"});
        table->addRow({"3", "Gamma",   "Active",   "85"});
        table->addRow({"4", "Delta",   "Pending",  "61"});
        table->addRow({"5", "Epsilon", "Active",   "92"});

        page->addChild(Card(table, "Records"));
    }

    // ── Calendar ──────────────────────────────────────────────────────────────
    {
        auto cal = std::make_shared<Calendar>();
        page->addChild(Card(cal, "Calendar"));
    }

    return page;
}

// ── Root layout ───────────────────────────────────────────────────────────────

static FlexNode::Ptr CreateGallery() {
    auto root = FlexNode::Column();
    root->style.backgroundColor = Theme::Background;
    root->style.setWidthFull();
    root->style.setHeightFull();

    // Header bar
    auto header = FlexNode::Row();
    header->style.backgroundColor = Theme::MenuBar;
    header->style.setWidthFull();
    header->style.setHeight(48);
    header->style.setPadding(12, 0);
    header->style.setGap(10);
    header->style.setAlignItems(YGAlignCenter);

    auto title = std::make_shared<TextNode>("MochiKit UI Gallery");
    title->fontSize = Theme::FontMedium;
    title->color    = Theme::TextPrimary;
    header->addChild(title);

    auto badge = std::make_shared<BadgeNode>("v0.1");
    badge->color     = Theme::Accent;
    badge->textColor = SK_ColorWHITE;
    header->addChild(badge);

    root->addChild(header);

    // Body (sidebar + content)
    auto body = FlexNode::Row();
    body->style.setFlex(1.0f);

    // Sidebar
    auto sidebar = FlexNode::Column();
    sidebar->style.backgroundColor = Theme::Sidebar;
    sidebar->style.setWidth(160);
    sidebar->style.setPadding(16);
    sidebar->style.setGap(8);

    auto themeLabel = SectionLabel("THEME");
    sidebar->addChild(themeLabel);

    auto mkThemeItem = [](const std::string& name, ThemeType type) {
        auto btn = std::make_shared<ButtonNode>();
        btn->label          = name;
        btn->useThemeColors = false;
        btn->normalColor    = SK_ColorTRANSPARENT;
        btn->textColor      = Theme::TextSecondary;
        btn->hoverColor     = Theme::HoverOverlay;
        btn->style.setWidthFull();
        btn->onClick = [type]() {
            ThemeSwitcher::getInstance().setTheme(type);
        };
        return btn;
    };

    sidebar->addChild(mkThemeItem("Dark",       ThemeType::Dark));
    sidebar->addChild(mkThemeItem("Light",      ThemeType::Light));
    sidebar->addChild(mkThemeItem("MD3 Dark",   ThemeType::Md3Dark));
    sidebar->addChild(mkThemeItem("MD3 Light",  ThemeType::Md3Light));
    sidebar->addChild(mkThemeItem("Minimal",    ThemeType::Minimal));
    sidebar->addChild(mkThemeItem("WinUI Dark", ThemeType::WinuiDark));
    sidebar->addChild(mkThemeItem("WinUI Light",ThemeType::WinuiLight));
    sidebar->addChild(mkThemeItem("System",     ThemeType::System));

    auto sep = std::make_shared<SeparatorNode>();
    sidebar->addChild(sep);

    auto accentDot = std::make_shared<FlexNode>();
    accentDot->style.setWidth(12);
    accentDot->style.setHeight(12);
    accentDot->style.backgroundColor = Theme::Accent;
    accentDot->style.borderRadius    = 6.0f;

    auto accentRow = FlexNode::Row();
    accentRow->style.setGap(8);
    accentRow->style.setAlignItems(YGAlignCenter);
    accentRow->addChild(accentDot);
    auto accentLbl = std::make_shared<TextNode>("Accent");
    accentLbl->fontSize = Theme::FontSmall;
    accentLbl->color    = Theme::TextSecondary;
    accentRow->addChild(accentLbl);
    sidebar->addChild(accentRow);

    body->addChild(sidebar);

    // Main tabs
    auto tabs = std::make_shared<TabsNode>();
    tabs->style.setFlex(1.0f);

    tabs->addTab("Controls",    Scrollable(CreateControlsTab()));
    tabs->addTab("Inputs",      Scrollable(CreateInputsTab()));
    tabs->addTab("Visualizers", Scrollable(CreateVisualizersTab()));
    tabs->addTab("Palette",     Scrollable(CreatePaletteTab()));
    tabs->addTab("Data",        Scrollable(CreateDataTab()));

    body->addChild(tabs);
    root->addChild(body);

    return root;
}

// ── Entry point ───────────────────────────────────────────────────────────────

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Application::getInstance().init();

    Win32Window window("MochiKit UI Gallery", 1280, 800);
    window.enableMica(true);

    auto menuBar = MenuBarFactory::Create(MenuBackend::Skia);
    menuBar->addMenu("File", {
        {"Exit", 101, []() { PostQuitMessage(0); }}
    });
    menuBar->addMenu("Theme", {
        {"Dark",       201, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::Dark);      }},
        {"Light",      202, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::Light);     }},
        {"MD3 Dark",   203, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::Md3Dark);   }},
        {"MD3 Light",  204, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::Md3Light);  }},
        {"Minimal",    205, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::Minimal);   }},
        {"WinUI Dark", 206, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::WinuiDark); }},
        {"WinUI Light",207, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::WinuiLight);}},
        {"System",     208, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::System);    }}
    });
    window.setMenuBar(std::move(menuBar));

    window.setRoot(CreateGallery());
    window.run();

    return 0;
}
