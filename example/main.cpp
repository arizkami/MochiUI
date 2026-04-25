#include <include/core/Application.hpp>
#include <include/gui/Components.hpp>
#include <include/gui/Layout.hpp>
#include <include/gui/MochiUI.h>
#include <include/gui/Theme.hpp>
#include <include/platform/windows/Window.hpp>
#include <include/utils/Misc/ThemeSwitcher.hpp>

using namespace MochiUI;

FlexNode::Ptr CreateHelloWorldUI() {
  auto root = FlexNode::Column();
  root->style.backgroundColor = Theme::Background;
  root->style.setWidthFull();
  root->style.setHeightFull();
  root->style.setPadding(20);
  root->style.setGap(20);

  // 1. Header
  auto header = std::make_shared<TextNode>("MochiUI Typography Test");
  header->fontSize = Theme::FontHeader;
  header->color = Theme::Accent;
  header->textAlign = TextAlign::Left;
  header->style.setPadding(10, 5);
  root->addChild(header);

  // 2. Multi-language Section
  auto typoSection = std::make_shared<GroupBox>();
  typoSection->title = "Multi-Language Support (Left Aligned with Padding)";
  typoSection->style.setPadding(20);
  typoSection->style.setGap(10);
  typoSection->style.backgroundColor = Theme::Card;
  typoSection->style.setWidthFull();

  auto addTypoSample = [&](std::string lang, std::string text, float size = Theme::FontNormal) {
    auto row = FlexNode::Row();
    row->style.setWidthFull();
    row->style.setHeight(Theme::ControlHeight + 10); // Fixed row height for better centering
    row->style.setGap(15);
    row->style.setAlignItems(YGAlignCenter); // Vertical center items in row

    auto label = std::make_shared<TextNode>(lang + ":");
    label->style.setWidth(100);
    label->color = Theme::TextSecondary;
    label->fontSize = Theme::FontSmall;
    label->textAlign = TextAlign::Left; // Revert to Left
    row->addChild(label);

    auto sample = std::make_shared<TextNode>(text);
    sample->fontSize = size;
    sample->color = Theme::TextPrimary;
    sample->textAlign = TextAlign::Left;
    // Test padding within the TextNode itself
    sample->style.setPadding(15, 5); 
    sample->style.backgroundColor = SkColorSetA(Theme::Accent, 20); // Show padding area
    sample->style.setFlex(1.0f); // Make the box flexible
    row->addChild(sample);

    typoSection->addChild(row);
  };

  addTypoSample("English", "The quick brown fox jumps over the lazy dog.");
  addTypoSample("Thai", "สวัสดีชาวโลก! นี่คือการทดสอบภาษาไทยใน MochiUI", Theme::FontMedium);
  addTypoSample("Japanese", "こんにちは世界！これは日本語のテストです。", Theme::FontMedium);
  addTypoSample("Arabic", "مرحبا بالعالم! هذا اختبار للغة العربية", Theme::FontMedium);
  addTypoSample("Emoji", "🚀 🦀 🦄 🌈 🍕 🍦 🎸 ⚡️", Theme::FontLarge);

  root->addChild(typoSection);

  // 3. Alignment Test
  auto alignSection = std::make_shared<GroupBox>();
  alignSection->title = "Alignment Tests";
  alignSection->style.setPadding(20);
  alignSection->style.setGap(10);
  alignSection->style.backgroundColor = Theme::Card;

  auto leftText = std::make_shared<TextNode>("Left Aligned Text");
  leftText->textAlign = TextAlign::Left;
  leftText->style.setWidthFull();
  leftText->style.backgroundColor = SkColorSetA(SK_ColorBLACK, 30);
  alignSection->addChild(leftText);

  auto centerText = std::make_shared<TextNode>("Center Aligned Text");
  centerText->textAlign = TextAlign::Center;
  centerText->style.setWidthFull();
  centerText->style.backgroundColor = SkColorSetA(SK_ColorBLACK, 30);
  alignSection->addChild(centerText);

  auto rightText = std::make_shared<TextNode>("Right Aligned Text");
  rightText->textAlign = TextAlign::Right;
  rightText->style.setWidthFull();
  rightText->style.backgroundColor = SkColorSetA(SK_ColorBLACK, 30);
  alignSection->addChild(rightText);

  root->addChild(alignSection);

  return root;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
                   int nCmdShow) {
  Application::getInstance().init();

  Win32Window window("MochiUI Typography Test", 1024, 768);
  window.enableMica(true);

  auto menuBar = MenuBarFactory::Create(MenuBackend::Skia);
  menuBar->addMenu("File", {{"Exit", 103, []() { PostQuitMessage(0); }}});

  menuBar->addMenu("View", {{"Light Theme", 201, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::Light); }},
                            {"Dark Theme", 202, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::Dark); }},
                            {"System Theme", 203, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::System); }},
                            {"Minimal Theme", 204, []() { ThemeSwitcher::getInstance().setTheme(ThemeType::Minimal); }}});

  window.setMenuBar(std::move(menuBar));

  window.setRoot(CreateHelloWorldUI());
  window.run();

  return 0;
}
