// Borderless floating music mini-player — AureliaUI example.
// Demonstrates: WindowMode::Borderless, startDrag(), center(), setMinSize(),
// setAlwaysOnTop(), close(), minimize(), ResourceManager + resources.xml icons.
#include <AUKGraphicInterface.hpp>
#include <AUKGraphicComponents.hpp>
#include <AUKPainter.hpp>
#include <BinaryResources.hpp>
#include <windows.h>
#include <chrono>
#include <string>
#include <memory>

using namespace AureliaUI;

// ── Palette ───────────────────────────────────────────────────────────────────
namespace P {
    constexpr AUKColor Bg        = AUKColor::RGB( 14,  14,  18);
    constexpr AUKColor Surface   = AUKColor::RGB( 22,  22,  28);
    constexpr AUKColor Card      = AUKColor::RGB( 30,  30,  38);
    constexpr AUKColor Border    = AUKColor::RGB(255, 255, 255, 55);
    constexpr AUKColor TextP     = AUKColor::RGB(240, 240, 248);
    constexpr AUKColor TextS     = AUKColor::RGB(150, 150, 165);
    constexpr AUKColor TextT     = AUKColor::RGB( 90,  90, 105);
    constexpr AUKColor Accent    = AUKColor::RGB(139,  92, 246);
    constexpr AUKColor AccentDim = AUKColor::RGB(139,  92, 246,  80);
    constexpr AUKColor Hover     = AUKColor::RGB(255, 255, 255,  28);
    constexpr AUKColor PinActive = AUKColor::RGB(139,  92, 246,  50);
}

// ── Track data ────────────────────────────────────────────────────────────────
struct Track { const char* title; const char* artist; const char* album; float dur; };
static const Track kTracks[] = {
    { "Midnight Echoes",   "Luna Voss",    "Neon Dusk",        214.f },
    { "Glass Horizon",     "The Parallax", "Tidal Shift",      187.f },
    { "Solstice",          "Aria Cole",    "Celestial",        243.f },
    { "Phantom Frequency", "Nexus Wave",   "Signal Loss",      198.f },
    { "Ember & Frost",     "Sable Coast",  "Winter Sessions",  229.f },
};
static constexpr int kTrackCount = 5;

// ── Playback state ────────────────────────────────────────────────────────────
static IWindow*                    gWindow   = nullptr;
static int                         gTrack    = 0;
static bool                        gPlaying  = false;
static bool                        gShuffle  = false;
static bool                        gRepeat   = false;
static float                       gProgress = 0.0f;
static float                       gVolume   = 0.72f;
static bool                        gPinned   = false;

static std::shared_ptr<TextNode>   gTitleLbl;
static std::shared_ptr<TextNode>   gArtistLbl;
static std::shared_ptr<TextNode>   gAlbumLbl;
static std::shared_ptr<TextNode>   gElapsedLbl;
static std::shared_ptr<TextNode>   gDurLbl;
static std::shared_ptr<SliderNode> gProgressSlider;
static std::shared_ptr<IconNode>   gPlayIcon;

static void refreshLabels() {
    const Track& t = kTracks[gTrack];
    if (gTitleLbl)  gTitleLbl->text  = t.title;
    if (gArtistLbl) gArtistLbl->text = t.artist;
    if (gAlbumLbl)  gAlbumLbl->text  = t.album;
    if (gProgressSlider) gProgressSlider->value = gProgress;
    if (gPlayIcon) gPlayIcon->setIcon(gPlaying ? "pause" : "play");

    char buf[12];
    float elapsed = gProgress * t.dur;
    snprintf(buf, sizeof(buf), "%d:%02d", (int)elapsed / 60, (int)elapsed % 60);
    if (gElapsedLbl) gElapsedLbl->text = buf;
    snprintf(buf, sizeof(buf), "%d:%02d", (int)t.dur / 60, (int)t.dur % 60);
    if (gDurLbl) gDurLbl->text = buf;
}

// ── Animated album art ────────────────────────────────────────────────────────
class AlbumArtNode : public FlexNode {
    float angle = 0.f;
    std::chrono::steady_clock::time_point last{std::chrono::steady_clock::now()};
public:
    AlbumArtNode() {
        style.setWidth(176.f);
        style.setHeight(176.f);
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }
    Size measure(Size) override { return {176.f, 176.f}; }

    void draw(SkCanvas* canvas) override {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;
        if (gPlaying) angle += dt * 20.f;

        float cx = frame.centerX(), cy = frame.centerY(), r = frame.width() * 0.5f;
        AUKPainter p(canvas);

        // Clip to disc boundary, draw everything inside
        p.save().clipCircle(cx, cy, r);

        // Disc base — radial gradient: soft purple centre fading to near-black
        p.radialGradient(cx, cy, r,
                         AUKColor::RGB(55, 18, 90),   // centre
                         AUKColor::RGB( 8,  4, 18));  // edge

        // Static concentric colour rings
        const AUKColor ringCol[] = {
            AUKColor::RGB(160,  80, 255, 60),
            AUKColor::RGB(120,  60, 200, 40),
            AUKColor::RGB( 80,  40, 160, 25),
        };
        for (int i = 0; i < 3; ++i)
            p.circleBorder(cx, cy, r * (0.72f - i * 0.22f), ringCol[i], r * 0.18f);

        // Spinning fine grooves
        p.save().rotate(angle, cx, cy);
        for (int i = 1; i <= 8; ++i)
            p.circleBorder(cx, cy, r * (0.35f + i * 0.065f),
                           AUKColor::RGB(200, 140, 255, (uint8_t)(10 + i * 7)), 0.6f);
        p.restore();

        // Inner label circle + spindle
        p.circle(cx, cy, r * 0.28f, P::Bg)
         .circle(cx, cy, r * 0.07f, P::AccentDim);

        p.restore(); // end disc clip

        // Outer accent ring (outside clip — drawn over the rounded edge)
        p.circleBorder(cx, cy, r - 1.f, P::AccentDim, 2.f);
    }

    bool needsRedraw() override { return gPlaying; }
};

// ── Chrome button (close / minimize / pin) ─────────────────────────────────────
class ChromeBtn : public FlexNode {
    AUKColor hoverBg;
    bool* activeFlag = nullptr;
    std::shared_ptr<IconNode> icon;
public:
    ChromeBtn(AUKColor hBg, const std::string& iconName, bool* flag = nullptr)
        : hoverBg(hBg), activeFlag(flag)
    {
        style.setWidth(26.f);
        style.setHeight(26.f);
        style.borderRadius = 13.f;
        style.setAlignItems(YGAlignCenter);
        style.setJustifyContent(YGJustifyCenter);
        enableHover = true;

        icon = std::make_shared<IconNode>();
        icon->setIcon(iconName);
        icon->color = P::TextS;
        icon->strokeWidth = 1.7f;
        icon->style.setWidth(13.f);
        icon->style.setHeight(13.f);
        addChild(icon);
    }

    void draw(SkCanvas* canvas) override {
        bool active = activeFlag && *activeFlag;
        AUKColor bg = active ? P::PinActive : (isHovered ? hoverBg : AUKColor::transparent());
        AUKPainter(canvas).roundRect(frame, style.borderRadius, bg);
        icon->color = active ? P::Accent : (isHovered ? P::TextP : P::TextS);
        drawChildren(canvas);
    }

    // Handle click directly — don't delegate to the IconNode child, which has
    // no onClick but would still consume the event via hitTest.
    bool onMouseDown(float x, float y) override {
        if (hitTest(x, y)) {
            isPressed = true;
            if (onClick) onClick();
            return true;
        }
        return false;
    }
};

// ── Drag bar — left-click on empty area drags the window ──────────────────────
class DragBar : public FlexNode {
public:
    DragBar() {
        style.setWidthFull();
        style.setHeight(40.f);
        style.setFlexShrink(0.f);
        style.setAlignItems(YGAlignCenter);
        style.setPadding(10.f, 0.f);
        style.flexDirection = FlexDirection::Row;
        style.setJustifyContent(YGJustifySpaceBetween);
    }

    void draw(SkCanvas* canvas) override {
        // Top corners rounded, bottom corners square (matches the window top edge)
        AUKPainter(canvas).roundRectCorners(frame, 10.f, 10.f, 0.f, 0.f, P::Surface);
        drawChildren(canvas);
    }

    // Pass clicks to children (buttons); drag on empty areas
    bool onMouseDown(float x, float y) override {
        for (auto it = children.rbegin(); it != children.rend(); ++it)
            if ((*it)->onMouseDown(x, y)) return true;
        if (hitTest(x, y) && gWindow) {
            gWindow->startDrag();
            return true;
        }
        return false;
    }
};

// ── Small icon button helper ───────────────────────────────────────────────────
// Button with an icon child. Overrides onMouseDown so the IconNode child
// (which has no onClick) cannot swallow the click before the parent fires it.
struct IconBtn : FlexNode {
    std::function<void()> action;

    static std::shared_ptr<IconBtn> make(const std::string& iconName, float iconPx,
                                         AUKColor col, std::function<void()> act) {
        auto btn = std::make_shared<IconBtn>();
        btn->action = act;
        btn->style.setWidth(iconPx + 14.f);
        btn->style.setHeight(iconPx + 14.f);
        btn->style.borderRadius = (iconPx + 14.f) * 0.5f;
        btn->style.setAlignItems(YGAlignCenter);
        btn->style.setJustifyContent(YGJustifyCenter);
        btn->enableHover = true;

        auto ic = std::make_shared<IconNode>();
        ic->setIcon(iconName);
        ic->color = col;
        ic->strokeWidth = 1.8f;
        ic->style.setWidth(iconPx);
        ic->style.setHeight(iconPx);
        btn->addChild(ic);
        return btn;
    }

    bool onMouseDown(float x, float y) override {
        if (hitTest(x, y)) {
            isPressed = true;
            if (action) action();
            return true;
        }
        return false;
    }
};

// ── Playlist row ──────────────────────────────────────────────────────────────
static FlexNode::Ptr PlaylistRow(int idx) {
    const Track& t = kTracks[idx];
    bool active = (idx == gTrack);

    auto row = FlexNode::Row();
    row->style.setHeight(36.f);
    row->style.setAlignItems(YGAlignCenter);
    row->style.setPadding(10.f, 0.f);
    row->style.setGap(8.f);
    row->style.borderRadius = 6.f;
    row->enableHover = true;
    if (active) row->style.backgroundColor = P::PinActive;

    auto num = std::make_shared<TextNode>();
    num->text     = active ? "\xe2\x96\xba" : std::to_string(idx + 1);
    num->fontSize = 10.f;
    num->color    = active ? P::Accent : P::TextT;
    num->style.setWidth(18.f);
    num->textAlign = TextAlign::Center;
    row->addChild(num);

    auto info = FlexNode::Column();
    info->style.setFlex(1.f);
    info->style.setGap(2.f);

    auto tit = std::make_shared<TextNode>();
    tit->text = t.title; tit->fontSize = 11.5f;
    tit->color = active ? P::TextP : P::TextS;
    info->addChild(tit);

    auto art = std::make_shared<TextNode>();
    art->text = t.artist; art->fontSize = 9.5f;
    art->color = P::TextT;
    info->addChild(art);

    row->addChild(info);

    char dur[8];
    snprintf(dur, sizeof(dur), "%d:%02d", (int)t.dur / 60, (int)t.dur % 60);
    auto durLbl = std::make_shared<TextNode>();
    durLbl->text = dur; durLbl->fontSize = 9.5f; durLbl->color = P::TextT;
    row->addChild(durLbl);

    row->onClick = [idx]() {
        gTrack = idx; gProgress = 0.f; gPlaying = true;
        refreshLabels();
        if (gWindow) gWindow->requestRedraw();
    };
    return row;
}

// ── Build the full UI ─────────────────────────────────────────────────────────
static FlexNode::Ptr CreateUI() {
    Theme::Background    = P::Bg;
    Theme::Accent        = P::Accent;
    Theme::TextPrimary   = P::TextP;
    Theme::TextSecondary = P::TextS;
    Theme::BorderRadius  = 8.f;

    auto root = FlexNode::Column();
    root->style.backgroundColor = P::Bg;
    root->style.borderRadius = 12.f;
    root->style.overflowHidden = true;

    // ── Title bar ─────────────────────────────────────────────────────────────
    auto bar = std::make_shared<DragBar>();

    auto left = FlexNode::Row();
    left->style.setGap(6.f);
    left->style.setAlignItems(YGAlignCenter);

    auto gripIc = std::make_shared<IconNode>();
    gripIc->setIcon("grip"); gripIc->color = P::TextT;
    gripIc->strokeWidth = 1.4f;
    gripIc->style.setWidth(14.f); gripIc->style.setHeight(14.f);
    left->addChild(gripIc);

    auto appLbl = std::make_shared<TextNode>("AureliaPlayer");
    appLbl->fontSize = 10.f; appLbl->color = P::TextT;
    left->addChild(appLbl);
    bar->addChild(left);

    auto chromeRow = FlexNode::Row();
    chromeRow->style.setGap(3.f);
    chromeRow->style.setAlignItems(YGAlignCenter);

    auto pinBtn = std::make_shared<ChromeBtn>(P::PinActive, "headphones", &gPinned);
    pinBtn->onClick = []() {
        gPinned = !gPinned;
        if (gWindow) gWindow->setAlwaysOnTop(gPinned);
    };
    chromeRow->addChild(pinBtn);

    auto minBtn = std::make_shared<ChromeBtn>(P::Hover, "minus");
    minBtn->onClick = []() { if (gWindow) gWindow->minimize(); };
    chromeRow->addChild(minBtn);

    auto closeBtn = std::make_shared<ChromeBtn>(AUKColor::RGB(180, 40, 40, 130), "x");
    closeBtn->onClick = []() { if (gWindow) gWindow->close(); };
    chromeRow->addChild(closeBtn);

    bar->addChild(chromeRow);
    root->addChild(bar);

    // ── Album art + track info ─────────────────────────────────────────────────
    auto artSection = FlexNode::Column();
    artSection->style.setAlignItems(YGAlignCenter);
    artSection->style.setPadding(16.f, 10.f);
    artSection->style.setGap(14.f);
    artSection->style.backgroundColor = P::Bg;

    artSection->addChild(std::make_shared<AlbumArtNode>());

    auto infoCol = FlexNode::Column();
    infoCol->style.setAlignItems(YGAlignCenter);
    infoCol->style.setGap(3.f);
    infoCol->style.setWidthFull();

    gTitleLbl = std::make_shared<TextNode>();
    gTitleLbl->fontSize = 14.f;
    gTitleLbl->color    = P::TextP;
    gTitleLbl->textAlign = TextAlign::Center;
    infoCol->addChild(gTitleLbl);

    gArtistLbl = std::make_shared<TextNode>();
    gArtistLbl->fontSize = 11.5f;
    gArtistLbl->color   = P::Accent;
    gArtistLbl->textAlign = TextAlign::Center;
    infoCol->addChild(gArtistLbl);

    gAlbumLbl = std::make_shared<TextNode>();
    gAlbumLbl->fontSize = 9.5f;
    gAlbumLbl->color   = P::TextT;
    gAlbumLbl->textAlign = TextAlign::Center;
    infoCol->addChild(gAlbumLbl);

    artSection->addChild(infoCol);
    root->addChild(artSection);

    // ── Progress ──────────────────────────────────────────────────────────────
    auto progSection = FlexNode::Column();
    progSection->style.setGap(4.f);
    progSection->style.setPadding(16.f, 4.f);

    gProgressSlider = std::make_shared<SliderNode>();
    gProgressSlider->style.setWidthFull();
    gProgressSlider->style.setHeight(20.f);
    gProgressSlider->value      = 0.f;
    gProgressSlider->trackColor = P::Card;
    gProgressSlider->fillColor  = P::Accent;
    gProgressSlider->thumbColor = P::TextP;
    gProgressSlider->trackHeight = 3;
    gProgressSlider->thumbRadius = 6;
    gProgressSlider->onValueChange = [](float v) { gProgress = v; refreshLabels(); };
    progSection->addChild(gProgressSlider);

    auto timeRow = FlexNode::Row();
    timeRow->style.setJustifyContent(YGJustifySpaceBetween);

    gElapsedLbl = std::make_shared<TextNode>("0:00");
    gElapsedLbl->fontSize = 9.5f; gElapsedLbl->color = P::TextT;
    timeRow->addChild(gElapsedLbl);

    gDurLbl = std::make_shared<TextNode>("0:00");
    gDurLbl->fontSize = 9.5f; gDurLbl->color = P::TextT;
    timeRow->addChild(gDurLbl);

    progSection->addChild(timeRow);
    root->addChild(progSection);

    // ── Media controls ────────────────────────────────────────────────────────
    auto controls = FlexNode::Row();
    controls->style.setAlignItems(YGAlignCenter);
    controls->style.setJustifyContent(YGJustifyCenter);
    controls->style.setGap(6.f);
    controls->style.setPadding(16.f, 8.f);

    // Shuffle toggle
    auto shuffleBtn = IconBtn::make("shuffle", 15.f, P::TextS, []() {
        gShuffle = !gShuffle;
    });
    controls->addChild(shuffleBtn);

    // Previous
    controls->addChild(IconBtn::make("skip-back", 18.f, P::TextS, []() {
        gTrack = (gTrack - 1 + kTrackCount) % kTrackCount;
        gProgress = 0.f; refreshLabels();
        if (gWindow) gWindow->requestRedraw();
    }));

    // Play / Pause button (larger)
    // Play / Pause — larger circular button; use IconBtn::make so the icon
    // child cannot swallow the click before the action fires.
    auto playBtnNode = IconBtn::make("play", 22.f, P::TextP, []() {
        gPlaying = !gPlaying; refreshLabels();
    });
    playBtnNode->style.setWidth(50.f); playBtnNode->style.setHeight(50.f);
    playBtnNode->style.borderRadius = 25.f;
    playBtnNode->style.backgroundColor = P::Accent;
    // Capture the icon child so we can swap play/pause later
    gPlayIcon = std::static_pointer_cast<IconNode>(playBtnNode->children.front());
    controls->addChild(playBtnNode);

    // Next
    controls->addChild(IconBtn::make("skip-forward", 18.f, P::TextS, []() {
        gTrack = (gTrack + 1) % kTrackCount;
        gProgress = 0.f; refreshLabels();
        if (gWindow) gWindow->requestRedraw();
    }));

    // Repeat toggle
    controls->addChild(IconBtn::make("repeat", 15.f, P::TextS, []() {
        gRepeat = !gRepeat;
    }));

    root->addChild(controls);

    // ── Volume row ────────────────────────────────────────────────────────────
    auto volRow = FlexNode::Row();
    volRow->style.setAlignItems(YGAlignCenter);
    volRow->style.setGap(8.f);
    volRow->style.setPadding(16.f, 8.f);

    auto volIc = std::make_shared<IconNode>();
    volIc->setIcon("volume-2"); volIc->color = P::TextT;
    volIc->strokeWidth = 1.5f;
    volIc->style.setWidth(15.f); volIc->style.setHeight(15.f);
    volRow->addChild(volIc);

    auto volSlider = std::make_shared<SliderNode>();
    volSlider->style.setFlex(1.f);
    volSlider->style.setHeight(20.f);
    volSlider->value      = gVolume;
    volSlider->trackColor = P::Card;
    volSlider->fillColor  = P::TextT;
    volSlider->thumbColor = P::TextS;
    volSlider->trackHeight = 3;
    volSlider->thumbRadius = 5;
    volSlider->onValueChange = [](float v) { gVolume = v; };
    volRow->addChild(volSlider);

    root->addChild(volRow);

    // ── Spacer ────────────────────────────────────────────────────────────────
    auto spacer = FlexNode::Create();
    spacer->style.setHeight(8.f);
    root->addChild(spacer);

    // ── Separator ─────────────────────────────────────────────────────────────
    auto sep = FlexNode::Create();
    sep->style.setHeight(1.f); sep->style.setWidthFull();
    sep->style.backgroundColor = P::Border;
    root->addChild(sep);

    // ── Playlist header ───────────────────────────────────────────────────────
    auto plHead = FlexNode::Row();
    plHead->style.setHeight(28.f);
    plHead->style.setPadding(12.f, 0.f);
    plHead->style.setAlignItems(YGAlignCenter);
    plHead->style.backgroundColor = P::Surface;

    auto plLbl = std::make_shared<TextNode>("UP NEXT");
    plLbl->fontSize = 9.5f; plLbl->color = P::TextT;
    plHead->addChild(plLbl);
    root->addChild(plHead);

    // ── Playlist ──────────────────────────────────────────────────────────────
    auto playlist = FlexNode::Column();
    playlist->style.setPadding(6.f, 6.f);
    playlist->style.setGap(2.f);
    for (int i = 0; i < kTrackCount; ++i)
        playlist->addChild(PlaylistRow(i));
    root->addChild(playlist);

    refreshLabels();
    return root;
}

// ── Playback tick timer ───────────────────────────────────────────────────────
static void CALLBACK PlaybackTick(HWND, UINT, UINT_PTR, DWORD) {
    if (!gPlaying) return;
    const Track& t = kTracks[gTrack];
    gProgress += 1.f / t.dur;
    if (gProgress >= 1.f) {
        gProgress = 0.f;
        if (!gRepeat)
            gTrack = (gTrack + 1) % kTrackCount;
    }
    refreshLabels();
    if (gWindow) gWindow->requestRedraw();
}

// ── Entry point ───────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Application::getInstance().init();
    InitBinaryResources();   // loads all res:// SVG icons from resources.xml

    ThemeSwitcher::getInstance().setTheme(ThemeType::Dark);

    Window window("AureliaPlayer", 400, 750);
    gWindow = &window;

    window.setDarkMode(true);
    window.setWindowMode(WindowMode::Borderless);
    window.setCornerPreference(CornerPreference::Round);
    window.setShadow(true);
    window.setMinSize(280, 500);
    window.center();

    window.setRoot(CreateUI());

    SetTimer((HWND)window.getNativeHandle(), 10, 1000, PlaybackTick);

    window.run();
    return 0;
}
