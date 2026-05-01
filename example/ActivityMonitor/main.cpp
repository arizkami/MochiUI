#include <AUKApplication.hpp>
#include <AUKGraphicInterface.hpp>
#include <AUKGraphicComponents.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <map>

using namespace AureliaUI;

// ─── macOS Sonoma dark palette ────────────────────────────────────────────────
namespace Mac {
    static constexpr SkColor WinBg      = SkColorSetRGB( 28,  28,  30);
    static constexpr SkColor ToolbarBg  = SkColorSetRGB( 36,  36,  38);
    static constexpr SkColor HeaderBg   = SkColorSetRGB( 32,  32,  34);
    static constexpr SkColor BottomBg   = SkColorSetRGB( 22,  22,  24);
    static constexpr SkColor Sep        = SkColorSetRGB( 54,  54,  58);
    static constexpr SkColor RowEven    = SkColorSetARGB( 14, 255, 255, 255);
    static constexpr SkColor RowSel     = SkColorSetARGB( 55,  10, 132, 255);
    static constexpr SkColor CardBg     = SkColorSetARGB( 55,  50,  50,  54);
    static constexpr SkColor SegBg      = SkColorSetARGB( 90,  50,  50,  54);
    static constexpr SkColor SegActive  = SkColorSetARGB(210,  62,  62,  68);
    static constexpr SkColor TextP      = SkColorSetRGB(242, 242, 247);
    static constexpr SkColor TextS      = SkColorSetRGB(174, 174, 178);
    static constexpr SkColor TextT      = SkColorSetRGB( 99,  99, 102);
    static constexpr SkColor Blue       = SkColorSetRGB( 10, 132, 255);
    static constexpr SkColor Green      = SkColorSetRGB( 48, 209,  88);
    static constexpr SkColor Orange     = SkColorSetRGB(255, 159,  10);
    static constexpr SkColor Red        = SkColorSetRGB(255,  69,  58);
}

// ─── Process data ─────────────────────────────────────────────────────────────
struct ProcessInfo {
    uint32_t    pid;
    uint32_t    threads;
    std::string name;
    float       cpuUsage;
    uint64_t    memoryUsage;
};

class ProcessProvider {
public:
    struct ProcState { ULONGLONG lastTime, lastProcTime; };

    std::vector<ProcessInfo> getProcesses() {
        std::vector<ProcessInfo> list;
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap == INVALID_HANDLE_VALUE) return list;
        PROCESSENTRY32W pe; pe.dwSize = sizeof(pe);
        if (Process32FirstW(hSnap, &pe)) {
            do {
                ProcessInfo info;
                info.pid     = pe.th32ProcessID;
                info.threads = pe.cntThreads;
                char name[MAX_PATH];
                WideCharToMultiByte(CP_UTF8, 0, pe.szExeFile, -1, name, MAX_PATH, NULL, NULL);
                info.name = name;
                HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.pid);
                if (hProc) {
                    PROCESS_MEMORY_COUNTERS pmc;
                    if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc)))
                        info.memoryUsage = pmc.WorkingSetSize;
                    FILETIME ct, et, kt, ut;
                    if (GetProcessTimes(hProc, &ct, &et, &kt, &ut)) {
                        ULARGE_INTEGER k, u;
                        k.LowPart = kt.dwLowDateTime; k.HighPart = kt.dwHighDateTime;
                        u.LowPart = ut.dwLowDateTime; u.HighPart = ut.dwHighDateTime;
                        auto now = (ULONGLONG)std::chrono::steady_clock::now().time_since_epoch().count();
                        ULONGLONG pt = k.QuadPart + u.QuadPart;
                        if (states.count(info.pid)) {
                            auto dp = pt - states[info.pid].lastProcTime;
                            info.cpuUsage = (float)dp / 100000.0f;
                            if (info.cpuUsage > 100.0f) info.cpuUsage = 0.1f;
                        } else info.cpuUsage = 0.0f;
                        states[info.pid] = { now, pt };
                    }
                    CloseHandle(hProc);
                } else { info.memoryUsage = 0; info.cpuUsage = 0; }
                list.push_back(info);
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
        if (states.size() > 1000) states.clear();
        return list;
    }
private:
    std::map<uint32_t, ProcState> states;
};

// ─── Global UI state ──────────────────────────────────────────────────────────
std::shared_ptr<GraphNode>  cpuHistGraph;
std::shared_ptr<GraphNode>  memHistGraph;
std::shared_ptr<FlexNode>   processListContainer;
std::shared_ptr<TextNode>   cpuPctLabel;
std::shared_ptr<TextNode>   userCpuLabel;
std::shared_ptr<TextNode>   sysCpuLabel;
std::shared_ptr<TextNode>   idleCpuLabel;
std::shared_ptr<TextNode>   procCountLabel;
std::shared_ptr<TextNode>   memUsedLabel;
std::shared_ptr<ProgressBar> cpuBar;
std::shared_ptr<ProgressBar> memBar;


// ─── macOS segment tab button ─────────────────────────────────────────────────
class MacSegTab : public FlexNode {
public:
    std::string label;
    bool        active = false;

    MacSegTab(const std::string& lbl, bool isActive = false) : label(lbl), active(isActive) {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
    }

    Size measure(Size) override {
        float tw = FontManager::getInstance().measureText(label, 12.0f);
        float w = tw + 20.0f;
        if (style.widthMode == SizingMode::Fixed) w = style.width;
        return { w, 28.0f };
    }

    void draw(SkCanvas* canvas) override {
        SkPaint p; p.setAntiAlias(true);

        if (active) {
            p.setColor(Mac::SegActive);
            canvas->drawRoundRect(frame, 5.0f, 5.0f, p);

            // Bottom accent bar
            p.setColor(Mac::Blue);
            float inset = 10.0f;
            canvas->drawRoundRect(
                SkRect::MakeXYWH(frame.left() + inset, frame.bottom() - 2.0f,
                                 frame.width() - inset * 2, 2.0f),
                1.0f, 1.0f, p);
        } else if (isHovered) {
            p.setColor(SkColorSetARGB(22, 255, 255, 255));
            canvas->drawRoundRect(frame, 5.0f, 5.0f, p);
        }

        SkFontMetrics fm{};
        FontManager::getInstance().getFontMetrics(12.0f, &fm);
        float tw = FontManager::getInstance().measureText(label, 12.0f);
        float tx = frame.centerX() - tw / 2.0f;
        float ty = frame.centerY() - (fm.fAscent + fm.fDescent) / 2.0f - 1.0f;

        p.setColor(active ? Mac::TextP : Mac::TextS);
        FontManager::getInstance().drawText(canvas, label, tx, ty, 12.0f, p);
    }
};

// ─── 1-px horizontal separator ───────────────────────────────────────────────
class HSep : public FlexNode {
public:
    HSep() {
        style.setHeight(1.0f);
        style.setWidthFull();
        style.backgroundColor = Mac::Sep;
    }
};

// ─── 1-px vertical separator ─────────────────────────────────────────────────
class VSep : public FlexNode {
public:
    VSep() {
        style.setWidth(1.0f);
        style.setHeightFull();
        style.backgroundColor = Mac::Sep;
    }
};

// ─── Column header cell ───────────────────────────────────────────────────────
class ColHeaderCell : public FlexNode {
public:
    std::string text;
    bool        sortActive = false;

    ColHeaderCell(const std::string& t, bool sort = false) : text(t), sortActive(sort) {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
    }

    Size measure(Size available) override {
        float w = (style.widthMode == SizingMode::Fixed) ? style.width : available.width;
        return { w, 26.0f };
    }

    void draw(SkCanvas* canvas) override {
        if (isHovered) {
            SkPaint hp; hp.setAntiAlias(true);
            hp.setColor(SkColorSetARGB(18, 255, 255, 255));
            canvas->drawRect(frame, hp);
        }
        std::string display = sortActive ? text + " \xe2\x96\xbe" : text;
        SkFontMetrics fm{};
        FontManager::getInstance().getFontMetrics(11.0f, &fm);
        float ty = frame.centerY() - (fm.fAscent + fm.fDescent) / 2.0f;
        SkPaint p; p.setAntiAlias(true);
        p.setColor(sortActive ? Mac::TextP : Mac::TextS);
        FontManager::getInstance().drawText(canvas, display, frame.left() + 8.0f, ty, 11.0f, p);
    }
};

// ─── Process row ──────────────────────────────────────────────────────────────
static FlexNode::Ptr MakeProcessRow(const ProcessInfo& info, int idx) {
    auto row = FlexNode::Row();
    row->style.setHeight(26.0f);
    row->style.setAlignItems(YGAlignCenter);
    if (idx % 2 == 1) row->style.backgroundColor = Mac::RowEven;
    row->enableHover = true;

    auto cell = [](const std::string& txt, float w, bool flex, SkColor col, float fs = 12.5f) {
        auto t = std::make_shared<TextNode>();
        t->text = txt; t->fontSize = fs; t->color = col;
        t->style.setPadding(8.0f, 0);
        if (flex) t->style.setFlex(1.0f);
        else      t->style.setWidth(w);
        return t;
    };

    row->addChild(cell(info.name, 0, true, Mac::TextP));

    char cpuBuf[16]; sprintf(cpuBuf, "%.1f", info.cpuUsage);
    SkColor cpuCol = info.cpuUsage > 30.0f ? Mac::Red
                   : info.cpuUsage > 10.0f ? Mac::Orange
                   : Mac::TextP;
    row->addChild(cell(cpuBuf, 68, false, cpuCol));
    row->addChild(cell(std::to_string(info.threads), 62, false, Mac::TextS));
    row->addChild(cell(std::to_string(info.pid),     62, false, Mac::TextS));

    char memBuf[32];
    float memMB = (float)info.memoryUsage / (1024.0f * 1024);
    if (memMB >= 1024.0f) sprintf(memBuf, "%.1f GB", memMB / 1024.0f);
    else                  sprintf(memBuf, "%.0f MB",  memMB);
    row->addChild(cell(memBuf, 82, false, Mac::TextS));

    return row;
}

// ─── Stat row (colored dot + label + value) ───────────────────────────────────
static FlexNode::Ptr MakeStatRow(const std::string& lbl, SkColor dotColor,
                                 std::shared_ptr<TextNode>& outValue) {
    auto row = FlexNode::Row();
    row->style.setHeight(17.0f);
    row->style.setAlignItems(YGAlignCenter);
    row->style.setGap(6.0f);

    auto dot = FlexNode::Create();
    dot->style.setWidth(7.0f); dot->style.setHeight(7.0f);
    dot->style.backgroundColor = dotColor;
    dot->style.borderRadius = 3.5f;
    row->addChild(dot);

    auto lNode = std::make_shared<TextNode>();
    lNode->text = lbl; lNode->fontSize = 11.0f; lNode->color = Mac::TextS;
    row->addChild(lNode);

    auto vNode = std::make_shared<TextNode>();
    vNode->text = "—"; vNode->fontSize = 11.0f; vNode->color = Mac::TextP;
    vNode->style.setFlex(1.0f);
    row->addChild(vNode);
    outValue = vNode;

    return row;
}

// ─── Timer – refresh process list + metrics ───────────────────────────────────
void CALLBACK TimerProc(HWND hwnd, UINT, UINT_PTR, DWORD) {
    static ProcessProvider provider;
    auto procs = provider.getProcesses();
    std::sort(procs.begin(), procs.end(),
        [](const ProcessInfo& a, const ProcessInfo& b) { return a.cpuUsage > b.cpuUsage; });

    float totalCpu = 0.0f;
    for (auto& p : procs) totalCpu += p.cpuUsage;
    totalCpu = std::min(totalCpu, 100.0f);

    if (processListContainer) {
        processListContainer->removeAllChildren();
        const size_t n = std::min(procs.size(), (size_t)60);
        for (size_t i = 0; i < n; ++i)
            processListContainer->addChild(MakeProcessRow(procs[i], (int)i));
    }

    if (cpuHistGraph)  cpuHistGraph->addValue(totalCpu / 100.0f);
    if (cpuBar)        cpuBar->value = totalCpu / 100.0f;

    char buf[48];
    if (cpuPctLabel)  { sprintf(buf, "%.0f%%",           totalCpu);          cpuPctLabel->text  = buf; }
    if (userCpuLabel) { sprintf(buf, "%.1f%%",           totalCpu * 0.65f);  userCpuLabel->text = buf; }
    if (sysCpuLabel)  { sprintf(buf, "%.1f%%",           totalCpu * 0.35f);  sysCpuLabel->text  = buf; }
    if (idleCpuLabel) { sprintf(buf, "%.1f%%",           100.0f - totalCpu); idleCpuLabel->text = buf; }
    if (procCountLabel){ sprintf(buf, "%d processes",    (int)procs.size()); procCountLabel->text = buf; }

    MEMORYSTATUSEX mem; mem.dwLength = sizeof(mem); GlobalMemoryStatusEx(&mem);
    float ramFrac = (float)mem.dwMemoryLoad / 100.0f;
    if (memHistGraph) memHistGraph->addValue(ramFrac);
    if (memBar)       memBar->value = ramFrac;
    if (memUsedLabel) {
        sprintf(buf, "%.1f / %.0f GB",
            (float)(mem.ullTotalPhys - mem.ullAvailPhys) / (1024.0f * 1024 * 1024),
            (float)mem.ullTotalPhys  / (1024.0f * 1024 * 1024));
        memUsedLabel->text = buf;
    }

    InvalidateRect(hwnd, NULL, FALSE);
}

// ─── Build UI ─────────────────────────────────────────────────────────────────
static FlexNode::Ptr CreateUI() {
    // Apply macOS palette to the global theme
    Theme::Background    = Mac::WinBg;
    Theme::Sidebar       = Mac::ToolbarBg;
    Theme::Accent        = Mac::Blue;
    Theme::TextPrimary   = Mac::TextP;
    Theme::TextSecondary = Mac::TextS;
    Theme::BorderRadius  = 6.0f;
    Theme::FontSmall     = 11.0f;
    Theme::FontNormal    = 13.0f;

    auto root = FlexNode::Column();
    root->style.backgroundColor = Mac::WinBg;

    // ── Toolbar ────────────────────────────────────────────────────────────────
    auto toolbar = FlexNode::Row();
    toolbar->style.setHeight(44.0f);
    toolbar->style.backgroundColor = Mac::ToolbarBg;
    toolbar->style.setPadding(6.0f, 0);
    toolbar->style.setGap(8.0f);
    toolbar->style.setAlignItems(YGAlignCenter);
    toolbar->style.setFlexShrink(0.0f);

    auto appTitle = std::make_shared<TextNode>();
    appTitle->text = "Activity Monitor";
    appTitle->fontSize = 13.0f;
    appTitle->color = Mac::TextP;
    appTitle->style.setWidth(148.0f);
    toolbar->addChild(appTitle);

    // Segment control container
    auto segBg = FlexNode::Row();
    segBg->style.setFlex(1.0f);
    segBg->style.setHeight(32.0f);
    segBg->style.setAlignItems(YGAlignCenter);
    segBg->style.backgroundColor = Mac::SegBg;
    segBg->style.borderRadius = 8.0f;
    segBg->style.setPadding(3.0f);
    segBg->style.setGap(2.0f);

    const std::vector<std::string> tabLabels = { "CPU", "Memory", "Energy", "Disk", "Network" };
    std::vector<std::shared_ptr<MacSegTab>> segTabs;

    // Create all tabs first so onClick can capture the complete vector
    for (size_t i = 0; i < tabLabels.size(); ++i) {
        auto tab = std::make_shared<MacSegTab>(tabLabels[i], i == 0);
        tab->style.setFlex(1.0f);
        segTabs.push_back(tab);
        segBg->addChild(tab);
    }
    for (size_t i = 0; i < segTabs.size(); ++i) {
        auto thisTab = segTabs[i];
        auto allTabs = segTabs; // copy shared_ptr vector — safe to capture by value
        thisTab->onClick = [thisTab, allTabs]() {
            for (auto& t : allTabs) t->active = false;
            thisTab->active = true;
        };
    }
    toolbar->addChild(segBg);

    procCountLabel = std::make_shared<TextNode>();
    procCountLabel->text = " ";
    procCountLabel->fontSize = 11.0f;
    procCountLabel->color = Mac::TextT;
    procCountLabel->style.setWidth(108.0f);
    toolbar->addChild(procCountLabel);

    root->addChild(toolbar);
    root->addChild(std::make_shared<HSep>());

    // ── Column headers ─────────────────────────────────────────────────────────
    auto colHeader = FlexNode::Row();
    colHeader->style.setHeight(26.0f);
    colHeader->style.setFlexShrink(0.0f);
    colHeader->style.backgroundColor = Mac::HeaderBg;
    colHeader->style.setAlignItems(YGAlignCenter);

    auto addHdr = [&](const std::string& t, float w, bool flex, bool sort = false) {
        auto c = std::make_shared<ColHeaderCell>(t, sort);
        if (flex) c->style.setFlex(1.0f);
        else      c->style.setWidth(w);
        colHeader->addChild(c);
    };
    addHdr("Process Name", 0, true, true);
    addHdr("% CPU",    68, false);
    addHdr("Threads",  62, false);
    addHdr("PID",      62, false);
    addHdr("Memory",   82, false);

    root->addChild(colHeader);
    root->addChild(std::make_shared<HSep>());

    // ── Process list ───────────────────────────────────────────────────────────
    auto scroll = std::make_shared<ScrollAreaNode>();
    scroll->style.setFlex(1.0f);
    processListContainer = FlexNode::Column();
    scroll->setContent(processListContainer);
    root->addChild(scroll);

    root->addChild(std::make_shared<HSep>());

    // ── Bottom panel ───────────────────────────────────────────────────────────
    auto bottom = FlexNode::Row();
    bottom->style.setHeight(168.0f);
    bottom->style.setFlexShrink(0.0f);
    bottom->style.backgroundColor = Mac::BottomBg;
    bottom->style.setPadding(14.0f, 12.0f);
    bottom->style.setGap(14.0f);
    bottom->style.setAlignItems(YGAlignStretch);

    // ── Left: CPU stats ──────────────────────────────────────
    auto cpuStats = FlexNode::Column();
    cpuStats->style.setWidth(172.0f);
    cpuStats->style.setGap(5.0f);

    auto cpuSectionLabel = std::make_shared<TextNode>();
    cpuSectionLabel->text = "CPU USAGE";
    cpuSectionLabel->fontSize = 10.0f;
    cpuSectionLabel->color = Mac::TextT;
    cpuStats->addChild(cpuSectionLabel);

    cpuPctLabel = std::make_shared<TextNode>();
    cpuPctLabel->text = "0%";
    cpuPctLabel->fontSize = 28.0f;
    cpuPctLabel->color = Mac::TextP;
    cpuStats->addChild(cpuPctLabel);

    cpuBar = std::make_shared<ProgressBar>();
    cpuBar->style.setWidthFull();
    cpuBar->style.setHeight(5.0f);
    cpuBar->fillColor      = Mac::Green;
    cpuBar->backgroundColor = Mac::CardBg;
    cpuBar->borderRadius   = 2.5f;
    cpuStats->addChild(cpuBar);

    auto cpuGap = FlexNode::Create();
    cpuGap->style.setFlex(1.0f);
    cpuStats->addChild(cpuGap);

    cpuStats->addChild(MakeStatRow("User",   Mac::Blue,   userCpuLabel));
    cpuStats->addChild(MakeStatRow("System", Mac::Orange, sysCpuLabel));
    cpuStats->addChild(MakeStatRow("Idle",   Mac::TextT,  idleCpuLabel));

    bottom->addChild(cpuStats);
    bottom->addChild(std::make_shared<VSep>());

    // ── Center: CPU history graph ─────────────────────────────
    auto cpuGraphCol = FlexNode::Column();
    cpuGraphCol->style.setFlex(1.0f);
    cpuGraphCol->style.setGap(5.0f);

    auto cpuGraphLabel = std::make_shared<TextNode>();
    cpuGraphLabel->text = "CPU HISTORY";
    cpuGraphLabel->fontSize = 10.0f;
    cpuGraphLabel->color = Mac::TextT;
    cpuGraphCol->addChild(cpuGraphLabel);

    cpuHistGraph = std::make_shared<GraphNode>();
    cpuHistGraph->style.setFlex(1.0f);
    cpuHistGraph->style.setWidthFull();
    cpuHistGraph->lineColor  = Mac::Green;
    cpuHistGraph->fillColor  = AUKColor(Mac::Green).withAlpha(uint8_t(35));
    cpuHistGraph->maxPoints  = 60;
    cpuHistGraph->strokeWidth = 1.5f;
    cpuHistGraph->showGrid   = true;
    cpuGraphCol->addChild(cpuHistGraph);

    bottom->addChild(cpuGraphCol);
    bottom->addChild(std::make_shared<VSep>());

    // ── Right: Memory stats + history ────────────────────────
    auto memCol = FlexNode::Column();
    memCol->style.setWidth(172.0f);
    memCol->style.setGap(5.0f);

    auto memSectionLabel = std::make_shared<TextNode>();
    memSectionLabel->text = "MEMORY";
    memSectionLabel->fontSize = 10.0f;
    memSectionLabel->color = Mac::TextT;
    memCol->addChild(memSectionLabel);

    memUsedLabel = std::make_shared<TextNode>();
    memUsedLabel->text = "— GB";
    memUsedLabel->fontSize = 15.0f;
    memUsedLabel->color = Mac::TextP;
    memCol->addChild(memUsedLabel);

    memBar = std::make_shared<ProgressBar>();
    memBar->style.setWidthFull();
    memBar->style.setHeight(5.0f);
    memBar->fillColor       = Mac::Orange;
    memBar->backgroundColor = Mac::CardBg;
    memBar->borderRadius    = 2.5f;
    memCol->addChild(memBar);

    auto memGraphLabel = std::make_shared<TextNode>();
    memGraphLabel->text = "MEMORY PRESSURE";
    memGraphLabel->fontSize = 10.0f;
    memGraphLabel->color = Mac::TextT;
    memGraphLabel->style.setMargin(0.0f);
    memCol->addChild(memGraphLabel);

    memHistGraph = std::make_shared<GraphNode>();
    memHistGraph->style.setFlex(1.0f);
    memHistGraph->style.setWidthFull();
    memHistGraph->lineColor  = Mac::Orange;
    memHistGraph->fillColor  = AUKColor(Mac::Orange).withAlpha(uint8_t(35));
    memHistGraph->maxPoints  = 60;
    memHistGraph->strokeWidth = 1.5f;
    memHistGraph->showGrid   = true;
    memCol->addChild(memHistGraph);

    bottom->addChild(memCol);
    root->addChild(bottom);

    return root;
}

// ─── Entry point ──────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Application::getInstance().init();
    Win32Window window("Activity Monitor", 1060, 800);
    window.enableMica(true);
    window.setDarkMode(true);
    window.setRoot(CreateUI());
    SetTimer((HWND)window.getNativeHandle(), 1, 1000, TimerProc);
    window.run();
    return 0;
}
