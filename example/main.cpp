#include <include/core/Application.hpp>
#include <include/gui/Components.hpp>
#include <include/gui/Layout.hpp>
#include <include/gui/MochiUI.h>
#include <include/gui/Theme.hpp>
#include <include/platform/windows/Window.hpp>
#include <include/utils/Misc/ThemeSwitcher.hpp>

using namespace MochiUI;

FlexNode::Ptr CreateInteractiveTab() {
    auto content = FlexNode::Column();
    content->style.setPadding(20);
    content->style.setGap(20);

    auto btnGroup = std::make_shared<GroupBox>();
    btnGroup->title = "Buttons";
    btnGroup->style.setPadding(15);
    btnGroup->style.setGap(10);
    btnGroup->style.flexDirection = FlexDirection::Row;
    
    auto btn1 = std::make_shared<ButtonNode>();
    btn1->label = "Default";
    btnGroup->addChild(btn1);

    auto btn2 = std::make_shared<ButtonNode>();
    btn2->label = "Ghost";
    btn2->useThemeColors = false;
    btn2->normalColor = SK_ColorTRANSPARENT;
    btn2->textColor = Theme::Accent;
    btnGroup->addChild(btn2);
    
    content->addChild(btnGroup);

    auto toggleGroup = std::make_shared<GroupBox>();
    toggleGroup->title = "Toggles";
    toggleGroup->style.setPadding(15);
    toggleGroup->style.setGap(20);
    toggleGroup->style.flexDirection = FlexDirection::Row;

    auto cb = std::make_shared<CheckboxNode>();
    cb->label = "Checkbox";
    toggleGroup->addChild(cb);

    auto sw = std::make_shared<SwitchNode>();
    sw->label = "Switch";
    toggleGroup->addChild(sw);

    content->addChild(toggleGroup);

    return content;
}

FlexNode::Ptr CreateFormsTab() {
    auto content = FlexNode::Column();
    content->style.setPadding(20);
    content->style.setGap(20);

    auto inputs = std::make_shared<GroupBox>();
    inputs->title = "Text Inputs";
    inputs->style.setPadding(15);
    inputs->style.setGap(15);

    auto text = std::make_shared<TextInput>();
    text->placeholder = "Text input...";
    inputs->addChild(text);

    auto num = std::make_shared<NumberInput>();
    num->value = 42;
    num->style.setWidth(120);
    inputs->addChild(num);

    content->addChild(inputs);

    auto select = std::make_shared<GroupBox>();
    select->title = "Selection";
    select->style.setPadding(15);
    
    auto combo = std::make_shared<ComboBox>();
    combo->items = {"Option A", "Option B", "Option C"};
    combo->selectedIndex = 0;
    select->addChild(combo);

    content->addChild(select);

    return content;
}

FlexNode::Ptr CreateFeedbackTab() {
    auto content = FlexNode::Column();
    content->style.setPadding(20);
    content->style.setGap(20);

    auto prog = std::make_shared<ProgressBar>();
    prog->value = 0.3f;
    content->addChild(prog);

    auto slider = std::make_shared<SliderNode>();
    slider->value = 0.3f;
    slider->onValueChange = [prog](float v) { prog->value = v; };
    content->addChild(slider);

    auto accordion = std::make_shared<AccordionNode>();
    accordion->addItem("Section 1", std::make_shared<TextNode>("Content for section 1..."));
    accordion->addItem("Section 2", std::make_shared<TextNode>("Content for section 2..."));
    content->addChild(accordion);

    return content;
}

FlexNode::Ptr CreateDataTab() {
    auto content = FlexNode::Column();
    content->style.setPadding(20);
    content->style.setGap(20);

    auto table = std::make_shared<Table>();
    auto head = std::make_shared<TableHead>();
    head->addColumn("ID", 50);
    head->addColumn("Name", 150);
    head->addColumn("Value", -1); // Flex
    table->setHeader(head);
    table->addRow({"1", "Alpha", "100"});
    table->addRow({"2", "Beta", "200"});
    table->addRow({"3", "Gamma", "300"});
    content->addChild(table);

    auto cal = std::make_shared<Calendar>();
    content->addChild(cal);

    return content;
}

FlexNode::Ptr CreateUIGallery() {
    auto root = FlexNode::Column();
    root->style.backgroundColor = Theme::Background;
    root->style.setWidthFull();
    root->style.setHeightFull();

    auto tabs = std::make_shared<TabsNode>();
    tabs->style.setFlex(1.0f);
    
    tabs->addTab("Interactive", CreateInteractiveTab());
    tabs->addTab("Forms", CreateFormsTab());
    tabs->addTab("Feedback", CreateFeedbackTab());
    tabs->addTab("Data", CreateDataTab());

    root->addChild(tabs);
    return root;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
                   int nCmdShow) {
  Application::getInstance().init();

  Win32Window window("MochiUI Gallery", 1024, 768);
  window.enableMica(true);

  auto menuBar = MenuBarFactory::Create(MenuBackend::Skia);
  menuBar->addMenu("File", {{"Exit", 103, []() { PostQuitMessage(0); }}});
  menuBar->addMenu("Theme", {
      {"Light", 201, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::Light); }},
      {"Dark", 202, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::Dark); }},
      {"System", 203, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::System); }},
      {"Minimal", 204, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::Minimal); }}
  });
  window.setMenuBar(std::move(menuBar));

  window.setRoot(CreateUIGallery());
  window.run();

  return 0;
}
