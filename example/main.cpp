#include <include/gui/MochiUI.h>
#include <include/gui/Layout.hpp>
#include <include/gui/Components.hpp>
#include <include/gui/Theme.hpp>
#include <include/gui/ConfirmationDialog.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <include/utils/Misc/ThemeSwitcher.hpp>
#include <include/core/Application.hpp>
#include <include/platform/windows/Window.hpp>
#include <include/utils/Misc/FileDialog.hpp>

#include <BinaryResources.hpp>

using namespace MochiUI;

FlexNode::Ptr CreateAppUI(int width, int height) {
    auto root = FlexNode::Row();
    root->style.backgroundColor = Theme::Background;
    root->style.setWidthFull();
    root->style.setHeightFull();

    auto sidebar = FlexNode::Column();
    sidebar->style.setWidth(250);
    sidebar->style.setHeightFull();
    sidebar->style.backgroundColor = Theme::Sidebar;
    sidebar->style.setPadding(10);
    sidebar->style.setGap(5);

    for(int i=1; i<=8; ++i) {
        auto item = FlexNode::Row();
        item->style.setHeight(35);
        item->style.setWidthFull();
        item->style.setPadding(8);
        item->style.setGap(12);
        item->style.setAlignItems(YGAlignCenter);
        item->style.backgroundColor = Theme::Card;
        item->style.borderRadius = 6;
        item->enableHover = true;

        auto icon = std::make_shared<IconNode>();
        static const std::string icons[] = {"activity", "airplay", "alarm-clock", "apple", "archive", "award", "bell", "bookmark"};
        icon->setIcon("res://" + icons[(i-1) % 8] + ".svg");
        icon->color = Theme::Accent;
        icon->style.setWidth(18);
        icon->style.setHeight(18);
        item->addChild(icon);

        auto text = std::make_shared<TextNode>();
        text->text = "Mochi Item " + std::to_string(i);
        text->color = Theme::TextPrimary;
        text->fontSize = 14;
        item->addChild(text);

        sidebar->addChild(item);
    }

    auto mainContent = FlexNode::Column();
    mainContent->style.setFlex(1.0f);
    mainContent->style.setPadding(40);
    mainContent->style.setGap(20);

    auto header = std::make_shared<TextNode>();
    header->text = "MochiUI Framework";
    header->style.setPadding(5);
    header->color = Theme::TextPrimary;
    header->fontSize = 42;
    mainContent->addChild(header);

    auto hugContainer = FlexNode::Row();
    hugContainer->style.backgroundColor = Theme::Accent;
    hugContainer->style.setPadding(10);
    hugContainer->style.borderRadius = 10;
    hugContainer->style.setGap(10);
    hugContainer->style.setAlignItems(YGAlignCenter);

    auto starIcon = std::make_shared<IconNode>();
    starIcon->setIcon("res://sparkles.svg");
    starIcon->color = SK_ColorWHITE;
    starIcon->style.setWidth(20);
    starIcon->style.setHeight(20);
    hugContainer->addChild(starIcon);

    auto hugText = std::make_shared<TextNode>();
    hugText->text = "Yoga Layout v3.2 Engine";
    hugText->color = SK_ColorWHITE;
    hugContainer->addChild(hugText);
    mainContent->addChild(hugContainer);

    // Lucide Icons Preview
    auto iconSection = std::make_shared<GroupBox>();
    iconSection->title = "Lucide Icons Preview";
    iconSection->style.setPadding(20);
    iconSection->style.backgroundColor = Theme::Card;
    iconSection->style.borderRadius = 8;

    auto iconGrid = FlexNode::Row();
    iconGrid->style.setGap(15);
    iconGrid->style.setWidthFull();
    iconGrid->style.flexWrap = YGWrapWrap;

    static const std::vector<std::string> previewIcons = {
        "camera", "cloud", "cpu", "database", "fingerprint-pattern", "flask-conical", "gift", 
        "heart", "image", "key", "lamp", "languages", "map", "music", "palette", "phone",
        "rocket", "scissors", "settings", "shopping-cart", "smartphone", "star", "sun", "trash-2",
        "umbrella", "user", "video", "wifi", "zap"
    };

    for (const auto& iconName : previewIcons) {
        auto icon = std::make_shared<IconNode>();
        icon->setIcon("res://" + iconName + ".svg");
        icon->color = Theme::TextSecondary;
        icon->style.setWidth(24);
        icon->style.setHeight(24);
        icon->enableHover = true;
        icon->onClick = [icon]() { icon->color = Theme::Accent; };
        iconGrid->addChild(icon);
    }
    iconSection->addChild(iconGrid);
    mainContent->addChild(iconSection);

    auto checkboxSection = std::make_shared<GroupBox>();
    checkboxSection->title = "Checkbox Components";
    checkboxSection->style.setPadding(20);
    checkboxSection->style.setGap(12);
    checkboxSection->style.backgroundColor = Theme::Card;
    checkboxSection->style.borderRadius = 8;

    auto createCB = [](std::string label, bool checked) {
        auto cb = std::make_shared<CheckboxNode>();
        cb->label = label;
        cb->checked = checked;
        cb->style.setPadding(4);
        return cb;
    };
    checkboxSection->addChild(createCB("Enable dark mode", true));
    checkboxSection->addChild(createCB("Show notifications", false));
    checkboxSection->addChild(createCB("Auto-save changes", true));
    mainContent->addChild(checkboxSection);

    auto sliderSection = FlexNode::Column();
    sliderSection->style.setPadding(20);
    sliderSection->style.setGap(12);
    
    auto slider1 = std::make_shared<SliderNode>();
    slider1->style.setWidth(400);
    slider1->style.setHeight(30);
    sliderSection->addChild(slider1);
    mainContent->addChild(sliderSection);

    // --- New Components Section ---

    // 1. Inputs Section
    auto inputSection = std::make_shared<GroupBox>();
    inputSection->title = "Input Fields";
    inputSection->style.setPadding(20);
    inputSection->style.setGap(15);
    inputSection->style.backgroundColor = Theme::Card;
    inputSection->style.borderRadius = 8;

    auto textInput = std::make_shared<TextInput>();
    textInput->placeholder = "Enter your name...";
    textInput->style.setWidth(300);
    inputSection->addChild(textInput);

    auto numInput = std::make_shared<NumberInput>();
    numInput->value = 42.0;
    numInput->style.setWidth(150);
    inputSection->addChild(numInput);

    mainContent->addChild(inputSection);

    // 2. Table Section
    auto tableSection = std::make_shared<GroupBox>();
    tableSection->title = "Data Table";
    tableSection->style.setPadding(20);
    tableSection->style.backgroundColor = Theme::Card;
    tableSection->style.borderRadius = 8;

    auto table = std::make_shared<Table>();
    auto head = std::make_shared<TableHead>();
    head->addColumn("ID", 50);
    head->addColumn("Product", 200);
    head->addColumn("Price", 100);
    head->addColumn("Status", -1); // flex
    table->setHeader(head);

    table->addRow({"001", "Mochi Pro Laptop", "$1,299", "In Stock"});
    table->addRow({"002", "Yoga Wireless Mouse", "$45", "Low Stock"});
    table->addRow({"003", "Skia Graphics Tablet", "$299", "Out of Stock"});
    
    table->style.setWidthFull();
    tableSection->addChild(table);
    mainContent->addChild(tableSection);

    // 3. Pickers Section (Side by side)
    auto pickersRow = FlexNode::Row();
    pickersRow->style.setGap(20);
    pickersRow->style.setWidthFull();

    auto cpSection = std::make_shared<GroupBox>();
    cpSection->title = "Color Picker";
    cpSection->style.setPadding(20);
    cpSection->style.backgroundColor = Theme::Card;
    cpSection->style.borderRadius = 8;
    cpSection->addChild(std::make_shared<ColorPicker>());
    cpSection->style.setFlex(1.0f);
    pickersRow->addChild(cpSection);

    auto calSection = std::make_shared<GroupBox>();
    calSection->title = "Date Picker";
    calSection->style.setPadding(20);
    calSection->style.backgroundColor = Theme::Card;
    calSection->style.borderRadius = 8;
    calSection->addChild(std::make_shared<DatePicker>());
    calSection->style.setFlex(1.0f);
    pickersRow->addChild(calSection);

    mainContent->addChild(pickersRow);

    // 4. Dialogs Section
    auto dialogSection = std::make_shared<GroupBox>();
    dialogSection->title = "System Dialogs";
    dialogSection->style.setPadding(20);
    dialogSection->style.setGap(15);
    dialogSection->style.backgroundColor = Theme::Card;
    dialogSection->style.borderRadius = 8;

    auto dialogButtons = FlexNode::Row();
    dialogButtons->style.setGap(10);

    auto btnOpen = std::make_shared<ButtonNode>();
    btnOpen->label = "Open File";
    btnOpen->onClick = []() {
        auto path = FileDialog::OpenFile(GetActiveWindow(), L"Select a file", {{L"Text Files", L"*.txt"}, {L"All Files", L"*.*"}});
        if (!path.empty()) {
            ConfirmationDialog::showMessage(GetActiveWindow(), L"File Selected", path);
        }
    };
    dialogButtons->addChild(btnOpen);

    auto btnSave = std::make_shared<ButtonNode>();
    btnSave->label = "Save File";
    btnSave->onClick = []() {
        auto path = FileDialog::SaveFile(GetActiveWindow(), L"Save as...", {{L"JSON Files", L"*.json"}}, L"json");
        if (!path.empty()) {
            ConfirmationDialog::showMessage(GetActiveWindow(), L"File Saved", path);
        }
    };
    dialogButtons->addChild(btnSave);

    auto btnFolder = std::make_shared<ButtonNode>();
    btnFolder->label = "Select Folder";
    btnFolder->onClick = []() {
        auto path = FileDialog::SelectFolder(GetActiveWindow(), L"Choose a directory");
        if (!path.empty()) {
            ConfirmationDialog::showMessage(GetActiveWindow(), L"Folder Selected", path);
        }
    };
    dialogButtons->addChild(btnFolder);

    dialogSection->addChild(dialogButtons);
    mainContent->addChild(dialogSection);

    auto scrollArea = std::make_shared<ScrollAreaNode>();
    scrollArea->style.setFlex(1.0f);
    scrollArea->style.backgroundColor = Theme::Background;
    scrollArea->setContent(mainContent);

    root->addChild(sidebar);
    root->addChild(scrollArea);
    return root;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Application::getInstance().init();
    InitBinaryResources();
    Win32Window window("MochiUI Explorer", 1280, 800);
    window.enableMica(true);

    auto menuBar = MenuBarFactory::Create(MenuBackend::Skia);
    menuBar->addMenu("File", {
        { "Exit", 103, []() { PostQuitMessage(0); } }
    });
    menuBar->addMenu("Help", {
        { "Test Dialog", 201, []() { 
            ConfirmationDialog::showWarning(GetActiveWindow(), L"Warning", L"This is a test warning.", L"Do you want to proceed?");
        } },
        { "About MochiUI", 202, []() {
            ConfirmationDialog::showMessage(GetActiveWindow(), L"About", L"MochiUI Explorer v1.0", L"A high-performance UI framework powered by Skia and Yoga Layout.");
        } }
    });
    window.setMenuBar(std::move(menuBar));

    window.setRoot(CreateAppUI(1280, 800));
    window.run();
    return 0;
}
