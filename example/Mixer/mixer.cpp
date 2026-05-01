// Re-implementation of ref/index.html "Mixer Console" using AureliaUI (Skia + Yoga).
#include <AUKGraphicInterface.hpp>
#include <AUKGraphicComponents.hpp>

#include <windows.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <string>

using namespace AureliaUI;

namespace {

constexpr AUKColor kBg0            = AUKColor::RGB(0x0b, 0x11, 0x16);
constexpr AUKColor kHeaderBg       = AUKColor::RGB(0x1c, 0x27, 0x30);
constexpr AUKColor kSurface0       = AUKColor::RGB(0x12, 0x1b, 0x22);
constexpr AUKColor kSurface1       = AUKColor::RGB(0x16, 0x1f, 0x27);
constexpr AUKColor kSurfaceControls = AUKColor::RGB(0x18, 0x21, 0x28);
constexpr AUKColor kBar            = AUKColor::RGB(0x7f, 0x96, 0xa6);
constexpr AUKColor kBarText        = AUKColor::RGB(0x10, 0x14, 0x17);
constexpr AUKColor kText0          = AUKColor::RGB(0xd7, 0xe0, 0xe8);
constexpr AUKColor kText1          = AUKColor::RGB(0x9f, 0xb0, 0xbf);
constexpr AUKColor kText2          = AUKColor::RGB(0x71, 0x83, 0x94);
constexpr AUKColor kAccentCyan     = AUKColor::RGB(0x00, 0xa7, 0xc7);
constexpr AUKColor kAccentGreen    = AUKColor::RGB(0x66, 0xff, 0x00);
constexpr AUKColor kBorderSoft     = AUKColor::RGB(255, 255, 255, 45);
// Channel strip outer border (~ ref/index.html border-black/55)
constexpr AUKColor kStripBorder    = AUKColor::RGB(0, 0, 0, 140);

static std::string channelLabel(int i) {
    const int n = i + 1;
    return std::string("CH ") + (n < 10 ? "0" : "") + std::to_string(n);
}

static FlexNode::Ptr barRow(const char* leftLabel, SkColor dotColor, const char* rightGlyph) {
    auto row = FlexNode::Row();
    row->style.setHeight(24);
    row->style.backgroundColor = kBar;
    row->style.setPadding(6, 4);
    row->style.setAlignItems(YGAlignCenter);
    row->style.setJustifyContent(YGJustifySpaceBetween);

    auto left = FlexNode::Row();
    left->style.setGap(6);
    left->style.setAlignItems(YGAlignCenter);
    auto dot = std::make_shared<FlexNode>();
    dot->style.setWidth(8);
    dot->style.setHeight(8);
    dot->style.backgroundColor = dotColor;
    dot->style.borderRadius = 1;
    left->addChild(dot);
    auto t = std::make_shared<TextNode>(leftLabel);
    t->fontSize = 9;
    t->fontBold = true;
    t->color = kBarText;
    left->addChild(t);

    auto plus = std::make_shared<TextNode>(rightGlyph);
    plus->fontSize = 12;
    plus->color = kBarText;

    row->addChild(left);
    row->addChild(plus);
    return row;
}

static FlexNode::Ptr pluginRow(const char* name) {
    auto row = FlexNode::Row();
    row->style.setPadding(6, 6);
    row->style.setAlignItems(YGAlignCenter);
    row->style.setJustifyContent(YGJustifySpaceBetween);
    row->style.backgroundColor = kSurface1;
    row->enableHover = true;

    auto left = FlexNode::Row();
    left->style.setGap(6);
    left->style.setAlignItems(YGAlignCenter);
    left->style.setFlex(1.0f);
    auto mark = std::make_shared<FlexNode>();
    mark->style.setWidth(2);
    mark->style.setHeight(12);
    mark->style.backgroundColor = kAccentCyan;
    left->addChild(mark);
    auto nm = std::make_shared<TextNode>(name);
    nm->fontSize = 10;
    nm->color = kText0;
    left->addChild(nm);

    auto x = std::make_shared<TextNode>("x");
    x->fontSize = 10;
    x->color = AUKColor::RGB(255, 255, 255, 64);

    row->addChild(left);
    row->addChild(x);
    return row;
}

static FlexNode::Ptr sendRow(const char* name, const char* db, float level01) {
    auto col = FlexNode::Column();
    col->style.setPadding(6, 6);
    col->style.backgroundColor = kSurface1;
    col->enableHover = true;

    auto top = FlexNode::Row();
    top->style.setJustifyContent(YGJustifySpaceBetween);
    auto a = std::make_shared<TextNode>(name);
    a->fontSize = 9;
    a->color = kText1;
    auto b = std::make_shared<TextNode>(db);
    b->fontSize = 9;
    b->color = kText1;
    top->addChild(a);
    top->addChild(b);

    auto bar = std::make_shared<ProgressBar>();
    bar->style.setHeight(4);
    bar->style.setWidthFull();
    bar->backgroundColor = AUKColor::RGB(0, 0, 0, 140);
    bar->fillColor = level01 > 0.01f ? kAccentCyan : AUKColor::RGB(255, 255, 255, 36);
    bar->borderRadius = 2;
    bar->value = level01;

    col->addChild(top);
    col->addChild(bar);
    return col;
}

struct MixerStrip : FlexNode {
    int channelIndex = 0;
    std::shared_ptr<VUMeterNode> vu;
    std::shared_ptr<ButtonNode> muteBtn;
    std::shared_ptr<ButtonNode> soloBtn;
    bool muted = false;
    bool solo = false;
    float fakeLevel = 0;
    std::chrono::steady_clock::time_point lastDraw{};

    static std::shared_ptr<MixerStrip> create(int index) {
        auto strip = std::make_shared<MixerStrip>();
        strip->channelIndex = index;
        strip->lastDraw = std::chrono::steady_clock::now();
        strip->style.flexDirection = FlexDirection::Column;
        strip->style.setWidth(90);
        strip->style.setFlexShrink(0.0f);
        strip->style.setHeightFull();
        strip->style.backgroundColor = kSurface0;
        strip->style.borderRadius = 2;
        strip->style.overflowHidden = true;

        strip->addChild(barRow("INSERTS", kAccentGreen, "+"));
        auto inserts = FlexNode::Column();
        inserts->style.backgroundColor = kSurface1;
        inserts->style.setMinHeight(72);
        inserts->addChild(pluginRow("CLA-2A"));
        inserts->addChild(pluginRow("Pro-Q 4"));
        inserts->addChild(pluginRow("OTT"));
        strip->addChild(inserts);

        strip->addChild(barRow("SENDS", kAccentCyan, "+"));
        auto sends = FlexNode::Column();
        sends->style.backgroundColor = kSurface1;
        sends->style.setMinHeight(48);
        sends->addChild(sendRow("ValhallaRoom", "-12.4", 0.65f));
        sends->addChild(sendRow("Echoboy", "-inf", 0.0f));
        strip->addChild(sends);

        auto spacer = FlexNode::Create();
        spacer->style.setFlex(1.0f);
        spacer->style.backgroundColor = kSurface1;
        strip->addChild(spacer);

        auto controls = FlexNode::Column();
        controls->style.setPadding(8);
        controls->style.backgroundColor = kSurfaceControls;
        controls->style.setAlignItems(YGAlignCenter);
        controls->style.setGap(4);

        auto panCol = FlexNode::Column();
        panCol->style.setAlignItems(YGAlignCenter);
        auto pan = std::make_shared<KnobNode>();
        pan->knobSize = 36;
        pan->value = 0.5f;
        pan->showValue = false;
        pan->arcFillColor = kAccentCyan;
        pan->indicatorColor = kAccentCyan;
        pan->knobBodyColor = AUKColor::RGB(40, 48, 56);
        pan->knobRingColor = AUKColor::RGB(55, 64, 74);
        panCol->addChild(pan);

        auto lr = FlexNode::Row();
        lr->style.setWidth(36);
        lr->style.setJustifyContent(YGJustifySpaceBetween);
        auto l = std::make_shared<TextNode>("L");
        l->fontSize = 9;
        l->color = kText2;
        auto r = std::make_shared<TextNode>("R");
        r->fontSize = 9;
        r->color = kText2;
        lr->addChild(l);
        lr->addChild(r);
        panCol->addChild(lr);
        controls->addChild(panCol);

        auto ms = FlexNode::Row();
        ms->style.setGap(4);
        ms->style.setWidthFull();
        ms->style.setPadding(2, 0);
        strip->muteBtn = std::make_shared<ButtonNode>();
        strip->muteBtn->label = "M";
        strip->muteBtn->fontSize = 10;
        strip->muteBtn->useThemeColors = false;
        strip->muteBtn->normalColor = AUKColor::RGB(0, 0, 0, 72);
        strip->muteBtn->textColor = kText2;
        strip->muteBtn->hoverColor = AUKColor::RGB(255, 255, 255, 100);
        strip->muteBtn->style.setFlex(1.0f);
        {
            std::weak_ptr<MixerStrip> w = strip;
            strip->muteBtn->onClick = [w]() {
                if (auto s = w.lock()) {
                    s->toggleMute();
                }
            };
        }

        strip->soloBtn = std::make_shared<ButtonNode>();
        strip->soloBtn->label = "S";
        strip->soloBtn->fontSize = 10;
        strip->soloBtn->useThemeColors = false;
        strip->soloBtn->normalColor = AUKColor::RGB(0, 0, 0, 72);
        strip->soloBtn->textColor = kText2;
        strip->soloBtn->hoverColor = AUKColor::RGB(255, 255, 255, 100);
        strip->soloBtn->style.setFlex(1.0f);
        {
            std::weak_ptr<MixerStrip> w = strip;
            strip->soloBtn->onClick = [w]() {
                if (auto s = w.lock()) {
                    s->toggleSolo();
                }
            };
        }

        ms->addChild(strip->muteBtn);
        ms->addChild(strip->soloBtn);
        controls->addChild(ms);

        auto meterRow = FlexNode::Row();
        meterRow->style.setWidthFull();
        meterRow->style.setGap(8);
        meterRow->style.setPadding(4, 0);
        meterRow->style.setAlignItems(YGAlignCenter);
        meterRow->style.setJustifyContent(YGJustifyCenter);

        strip->vu = std::make_shared<VUMeterNode>();
        strip->vu->vertical = true;
        strip->vu->showNumber = false;
        strip->vu->meterWidth = 14;
        strip->vu->style.setWidth(24);
        strip->vu->style.setHeight(200);
        strip->vu->backgroundColor = AUKColor::RGB(15, 15, 17);
        strip->vu->greenColor = kAccentCyan;
        strip->vu->yellowColor = AUKColor::RGB(0xcd, 0xdc, 0x39);
        strip->vu->redColor = AUKColor::RGB(0xcc, 0x44, 0x22);

        auto fader = std::make_shared<SliderNode>();
        fader->vertical = true;
        fader->style.setWidth(36);
        fader->style.setHeight(200);
        fader->value = 0.55f;
        fader->trackColor = AUKColor::RGB(0, 0, 0, 140);
        fader->fillColor = kAccentCyan;
        fader->thumbColor = AUKColor::RGB(0xc9, 0xd6, 0xe6);
        fader->trackHeight = 4;
        fader->thumbRadius = 7;

        meterRow->addChild(strip->vu);
        meterRow->addChild(fader);
        controls->addChild(meterRow);

        strip->addChild(controls);

        auto track = std::make_shared<TextNode>(channelLabel(index));
        track->fontSize = 11;
        track->color = kBarText;
        track->textAlign = TextAlign::Center;
        track->style.backgroundColor = kBar;
        track->style.setPadding(8, 6);
        track->style.setWidthFull();
        strip->addChild(track);

        return strip;
    }

    void toggleMute() {
        muted = !muted;
        if (muted) {
            muteBtn->normalColor = AUKColor::RGB(239, 68, 68, 56);
            muteBtn->textColor = kText0;
        } else {
            muteBtn->normalColor = AUKColor::RGB(0, 0, 0, 72);
            muteBtn->textColor = kText2;
        }
        markDirty();
    }

    void toggleSolo() {
        solo = !solo;
        if (solo) {
            soloBtn->normalColor = AUKColor::RGB(0, 167, 199, 46);
            soloBtn->textColor = kText0;
        } else {
            soloBtn->normalColor = AUKColor::RGB(0, 0, 0, 72);
            soloBtn->textColor = kText2;
        }
        markDirty();
    }

    void tickMeter(float dt) {
        const float phase = static_cast<float>(channelIndex) * 0.37f;
        const float t = std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count();
        const float thresh = 0.82f - std::sin(phase + t * 0.3f) * 0.05f;

        if (muted) {
            fakeLevel = std::max(0.0f, fakeLevel - dt * 3.5f);
        } else {
            fakeLevel = std::max(0.0f, fakeLevel - dt * 2.2f);
            const float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            if (r > thresh) {
                fakeLevel = std::clamp(0.2f + r * 0.75f, 0.0f, 1.0f);
            }
        }
        if (vu) {
            vu->setValue(fakeLevel);
            vu->update(dt);
        }
    }

    void draw(SkCanvas* canvas) override {
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - lastDraw).count();
        lastDraw = now;
        tickMeter(std::min(dt, 0.1f));
        FlexNode::draw(canvas);

        SkPaint border;
        border.setAntiAlias(true);
        border.setStyle(SkPaint::kStroke_Style);
        border.setColor(kStripBorder);
        border.setStrokeWidth(1);
        const float rr = style.borderRadius;
        canvas->drawRoundRect(frame, rr, rr, border);
    }

    bool needsRedraw() override { return true; }
};

static FlexNode::Ptr buildMixerRoot(FlexNode::Ptr& outConsoleHint, std::shared_ptr<ButtonNode>& outConsoleBtn) {
    auto root = FlexNode::Column();
    root->style.setWidthFull();
    root->style.setHeightFull();
    root->style.backgroundColor = kBg0;

    auto header = FlexNode::Row();
    header->style.setWidthFull();
    header->style.setHeight(48);
    header->style.backgroundColor = kHeaderBg;
    header->style.setPadding(12, 10);
    header->style.setAlignItems(YGAlignCenter);
    header->style.setJustifyContent(YGJustifySpaceBetween);

    auto headerChrome = FlexNode::Column();
    headerChrome->style.setWidthFull();
    headerChrome->style.setFlexShrink(0.0f);

    auto left = FlexNode::Row();
    left->style.setGap(10);
    left->style.setAlignItems(YGAlignCenter);
    auto led = std::make_shared<FlexNode>();
    led->style.setWidth(8);
    led->style.setHeight(8);
    led->style.backgroundColor = kAccentGreen;
    led->style.borderRadius = 1;
    left->addChild(led);
    auto title = std::make_shared<TextNode>("MIXER CONSOLE");
    title->fontSize = 12;
    title->fontBold = true;
    title->color = kText1;
    left->addChild(title);
    auto badge = std::make_shared<TextNode>("16 CH");
    badge->fontSize = 10;
    badge->fontBold = true;
    badge->color = kText2;
    badge->style.setPadding(6, 2);
    badge->style.backgroundColor = AUKColor::RGB(0, 0, 0, 30);
    badge->style.borderRadius = 4;
    left->addChild(badge);

    outConsoleBtn = std::make_shared<ButtonNode>();
    outConsoleBtn->label = "CONSOLE";
    outConsoleBtn->fontSize = 11;
    outConsoleBtn->useThemeColors = false;
    outConsoleBtn->normalColor = AUKColor::RGB(30, 43, 53);
    outConsoleBtn->textColor = kText0;
    outConsoleBtn->hoverColor = AUKColor::RGB(38, 53, 66);
    outConsoleBtn->labelBold = true;

    header->addChild(left);
    header->addChild(outConsoleBtn);

    auto headerBorder = FlexNode::Create();
    headerBorder->style.setWidthFull();
    headerBorder->style.setHeight(1);
    headerBorder->style.backgroundColor = kBorderSoft;

    headerChrome->addChild(header);
    headerChrome->addChild(headerBorder);
    root->addChild(headerChrome);

    outConsoleHint = FlexNode::Create();
    outConsoleHint->style.setHeight(0);
    outConsoleHint->style.overflowHidden = true;
    auto hintText = std::make_shared<TextNode>(
        "Console view: 16 channel strips. Narrow window: wheel scrolls strips horizontally.");
    hintText->fontSize = 11;
    hintText->color = kText1;
    hintText->style.setPadding(8, 6);
    hintText->style.backgroundColor = AUKColor::RGB(10, 16, 22, 140);
    outConsoleHint->addChild(hintText);
    root->addChild(outConsoleHint);

    auto rack = FlexNode::Row();
    rack->style.setFlexShrink(0.0f);
    rack->style.setWidthAuto();
    rack->style.setPadding(12);
    rack->style.setGap(0);
    rack->style.setAlignItems(YGAlignStretch);

    for (int i = 0; i < 64; ++i) {
        rack->addChild(MixerStrip::create(i));
    }

    auto rackScroll = std::make_shared<ScrollAreaNode>();
    rackScroll->setHorizontal(true);
    rackScroll->setContent(rack);
    root->addChild(rackScroll);

    return root;
}

} // namespace

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Application::getInstance().init();
    ThemeSwitcher::getInstance().setTheme(ThemeType::Dark);
    srand(static_cast<unsigned>(GetTickCount()));

    Win32Window window("Mixer Console", 1440, 900);
    window.enableMica(true);

    FlexNode::Ptr consoleHint;
    std::shared_ptr<ButtonNode> consoleBtn;
    auto root = buildMixerRoot(consoleHint, consoleBtn);

    bool consoleOpen = false;
    consoleBtn->onClick = [&, root]() {
        consoleOpen = !consoleOpen;
        if (consoleOpen) {
            consoleHint->style.setHeightAuto();
            consoleBtn->normalColor = AUKColor::RGB(30, 76, 92);
        } else {
            consoleHint->style.setHeight(0);
            consoleBtn->normalColor = AUKColor::RGB(30, 43, 53);
        }
        root->markDirty();
    };

    window.setRoot(root);
    window.run();
    return 0;
}
