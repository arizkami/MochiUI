#include <include/gui/MochiUI.h>
#include <include/gui/Layout.hpp>
#include <include/gui/Components.hpp>
#include <include/gui/Theme.hpp>
#include <include/gui/ConfirmationDialog.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <include/utils/Misc/ThemeSwitcher.hpp>
#include <include/core/Application.hpp>
#include <include/platform/windows/Window.hpp>

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
        auto item = std::make_shared<TextNode>();
        item->text = "Mochi Item " + std::to_string(i);
        item->style.setHeight(35);
        item->style.setWidthFull();
        item->style.backgroundColor = Theme::Card;
        item->style.borderRadius = 6;
        item->color = Theme::TextPrimary;
        item->fontSize = 14;
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

    auto hugText = std::make_shared<TextNode>();
    hugText->text = "Yoga Layout v3.2 Engine";
    hugText->color = Theme::TextPrimary;
    hugContainer->addChild(hugText);
    mainContent->addChild(hugContainer);

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
    calSection->title = "Calendar";
    calSection->style.setPadding(20);
    calSection->style.backgroundColor = Theme::Card;
    calSection->style.borderRadius = 8;
    calSection->addChild(std::make_shared<Calendar>());
    calSection->style.setFlex(1.0f);
    pickersRow->addChild(calSection);

    mainContent->addChild(pickersRow);

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
