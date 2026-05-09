#include <SPHXApplication.hpp>
#include <SPHXGraphicInterface.hpp>
#include <SPHXGraphicComponents.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <map>
#include <algorithm>
#include <functional>

using namespace SphereUI;

// ─── macOS Sonoma dark palette ────────────────────────────────────────────────
namespace Mac {
    static constexpr SkColor WinBg      = SkColorSetRGB( 28,  28,  30);
    static constexpr SkColor ToolbarBg  = SkColorSetRGB( 36,  36,  38);
    static constexpr SkColor HeaderBg   = SkColorSetRGB( 32,  32,  34);
    static constexpr SkColor BottomBg   = SkColorSetRGB( 22,  22,  24);
    static constexpr SkColor Sep        = SkColorSetRGB( 54,  54,  58);
    static constexpr SkColor RowEven    = SkColorSetARGB( 14, 255, 255, 255);
    static constexpr SkColor RowSel     = SkColorSetARGB(100,  10, 132, 255);
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
static HWND     g_hwnd        = nullptr;
static std::string g_searchFilter;
static int      g_sortCol     = 0;     // 0=CPU, 1=Mem, 2=Name, 3=PID, 4=Threads
static bool     g_sortAsc     = false;
static uint32_t g_selectedPid = 0;
static int      g_activeTab   = 0;
static float    g_bottomHeight = 168.0f;
static bool     g_showIdleProcs = true;

std::shared_ptr<GraphNode>   cpuHistGraph;
std::shared_ptr<GraphNode>   memHistGraph;
std::shared_ptr<FlexNode>    processListContainer;
std::shared_ptr<TextNode>    cpuPctLabel;
std::shared_ptr<TextNode>    userCpuLabel;
std::shared_ptr<TextNode>    sysCpuLabel;
std::shared_ptr<TextNode>    idleCpuLabel;
std::shared_ptr<TextNode>    procCountLabel;
std::shared_ptr<TextNode>    memUsedLabel;
std::shared_ptr<ProgressBar> cpuBar;
std::shared_ptr<ProgressBar> memBar;
std::shared_ptr<BadgeNode>   g_cpuBadge;
std::shared_ptr<ButtonNode>  g_killBtn;
std::shared_ptr<FlexNode>    g_bottomPanel;
std::shared_ptr<FlexNode>    g_cpuContent;
std::shared_ptr<FlexNode>    g_memContent;
std::function<void(int)>     g_switchTab;

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

// ─── Separators ───────────────────────────────────────────────────────────────
class HSep : public FlexNode {
public:
    HSep() {
        style.setHeight(1.0f);
        style.setWidthFull();
        style.backgroundColor = Mac::Sep;
    }
};

class VSep : public FlexNode {
public:
    VSep() {
        style.setWidth(1.0f);
        style.setHeightFull();
        style.backgroundColor = Mac::Sep;
    }
};

// ─── Column header cell with live sort indicator ──────────────────────────────
class ColHeaderCell : public FlexNode {
public:
    std::string text;
    int         colIndex;

    ColHeaderCell(const std::string& t, int col) : text(t), colIndex(col) {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
        onClick = [this]() {
            if (g_sortCol == colIndex)
                g_sortAsc = !g_sortAsc;
            else {
                g_sortCol = colIndex;
                g_sortAsc = (colIndex == 2); // Name sorts ascending by default
            }
        };
    }

    Size measure(Size available) override {
        float w = (style.widthMode == SizingMode::Fixed) ? style.width : available.width;
        return { w, 26.0f };
    }

    void draw(SkCanvas* canvas) override {
        bool isSort = (g_sortCol == colIndex);
        if (isHovered) {
            SkPaint hp; hp.setAntiAlias(true);
            hp.setColor(SkColorSetARGB(18, 255, 255, 255));
            canvas->drawRect(frame, hp);
        }
        // ▴ = U+25B4, ▾ = U+25BE
        std::string display = text;
        if (isSort) display += g_sortAsc ? " \xe2\x96\xb4" : " \xe2\x96\xbe";

        SkFontMetrics fm{};
        FontManager::getInstance().getFontMetrics(11.0f, &fm);
        float ty = frame.centerY() - (fm.fAscent + fm.fDescent) / 2.0f;
        SkPaint p; p.setAntiAlias(true);
        p.setColor(isSort ? Mac::TextP : Mac::TextS);
        FontManager::getInstance().drawText(canvas, display, frame.left() + 8.0f, ty, 11.0f, p);
    }
};

// ─── Process row with selection highlight ─────────────────────────────────────
static FlexNode::Ptr MakeProcessRow(const ProcessInfo& info, int idx) {
    auto row = FlexNode::Row();
    row->style.setHeight(26.0f);
    row->style.setAlignItems(YGAlignCenter);

    bool isSelected = (info.pid == g_selectedPid);
    if (isSelected)
        row->style.backgroundColor = Mac::RowSel;
    else if (idx % 2 == 1)
        row->style.backgroundColor = Mac::RowEven;

    row->enableHover = true;
    uint32_t pid = info.pid;
    row->onClick = [pid]() {
        g_selectedPid = (g_selectedPid == pid) ? 0 : pid;
    };

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
    vNode->text = "\xe2\x80\x94"; vNode->fontSize = 11.0f; vNode->color = Mac::TextP;
    vNode->style.setFlex(1.0f);
    row->addChild(vNode);
    outValue = vNode;

    return row;
}

// ─── Timer – refresh process list + metrics ───────────────────────────────────
void CALLBACK TimerProc(HWND hwnd, UINT, UINT_PTR, DWORD) {
    static ProcessProvider provider;
    auto allProcs = provider.getProcesses();

    // Compute total CPU from all processes before any filtering
    float totalCpu = 0.0f;
    for (auto& p : allProcs) totalCpu += p.cpuUsage;
    totalCpu = std::min(totalCpu, 100.0f);

    // Build display list (filtered + sorted copy)
    auto procs = allProcs;

    if (!g_searchFilter.empty()) {
        std::string lo = g_searchFilter;
        std::transform(lo.begin(), lo.end(), lo.begin(), ::tolower);
        procs.erase(std::remove_if(procs.begin(), procs.end(), [&](const ProcessInfo& p) {
            std::string n = p.name;
            std::transform(n.begin(), n.end(), n.begin(), ::tolower);
            return n.find(lo) == std::string::npos;
        }), procs.end());
    }

    if (!g_showIdleProcs) {
        procs.erase(std::remove_if(procs.begin(), procs.end(), [](const ProcessInfo& p) {
            return p.cpuUsage < 0.01f && p.memoryUsage < 10ULL * 1024 * 1024;
        }), procs.end());
    }

    std::sort(procs.begin(), procs.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
        switch (g_sortCol) {
            case 1: return g_sortAsc ? a.memoryUsage < b.memoryUsage : a.memoryUsage > b.memoryUsage;
            case 2: return g_sortAsc ? a.name < b.name : a.name > b.name;
            case 3: return g_sortAsc ? a.pid < b.pid : a.pid > b.pid;
            case 4: return g_sortAsc ? a.threads < b.threads : a.threads > b.threads;
            default: return g_sortAsc ? a.cpuUsage < b.cpuUsage : a.cpuUsage > b.cpuUsage;
        }
    });

    if (processListContainer) {
        processListContainer->removeAllChildren();
        const size_t n = std::min(procs.size(), (size_t)80);
        for (size_t i = 0; i < n; ++i)
            processListContainer->addChild(MakeProcessRow(procs[i], (int)i));
    }

    if (cpuHistGraph) cpuHistGraph->addValue(totalCpu / 100.0f);
    if (cpuBar)       cpuBar->value = totalCpu / 100.0f;

    // Update CPU status badge
    if (g_cpuBadge) {
        if (totalCpu > 80.0f) {
            g_cpuBadge->text  = "HIGH";
            g_cpuBadge->nodeStyle.background = SPHXColor(Mac::Red);
        } else if (totalCpu > 50.0f) {
            g_cpuBadge->text  = "BUSY";
            g_cpuBadge->nodeStyle.background = SPHXColor(Mac::Orange);
        } else {
            g_cpuBadge->text  = "OK";
            g_cpuBadge->nodeStyle.background = SPHXColor(Mac::Green);
        }
    }

    // Update Kill button appearance based on selection
    if (g_killBtn) {
        if (g_selectedPid != 0) {
            g_killBtn->nodeStyle.background = SPHXColor(Mac::Red).withAlpha(uint8_t(200));
            g_killBtn->nodeStyle.foreground = SPHXColor::white();
        } else {
            g_killBtn->nodeStyle.background = SPHXColor::RGB(50, 50, 54);
            g_killBtn->nodeStyle.foreground = SPHXColor(Mac::TextT);
        }
    }

    char buf[64];
    if (cpuPctLabel)   { sprintf(buf, "%.0f%%",           totalCpu);          cpuPctLabel->text  = buf; }
    if (userCpuLabel)  { sprintf(buf, "%.1f%%",           totalCpu * 0.65f);  userCpuLabel->text = buf; }
    if (sysCpuLabel)   { sprintf(buf, "%.1f%%",           totalCpu * 0.35f);  sysCpuLabel->text  = buf; }
    if (idleCpuLabel)  { sprintf(buf, "%.1f%%",           100.0f - totalCpu); idleCpuLabel->text = buf; }
    if (procCountLabel) {
        if (!g_searchFilter.empty() || !g_showIdleProcs)
            sprintf(buf, "%d of %d", (int)procs.size(), (int)allProcs.size());
        else
            sprintf(buf, "%d processes", (int)allProcs.size());
        procCountLabel->text = buf;
    }

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

    // Segment control
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
    for (size_t i = 0; i < tabLabels.size(); ++i) {
        auto tab = std::make_shared<MacSegTab>(tabLabels[i], i == 0);
        tab->style.setFlex(1.0f);
        segTabs.push_back(tab);
        segBg->addChild(tab);
    }
    for (size_t i = 0; i < segTabs.size(); ++i) {
        auto thisTab = segTabs[i];
        auto allTabs = segTabs;
        int  idx     = (int)i;
        thisTab->onClick = [thisTab, allTabs, idx]() {
            for (auto& t : allTabs) t->active = false;
            thisTab->active = true;
            if (g_switchTab) g_switchTab(idx);
        };
    }
    toolbar->addChild(segBg);

    // Force Quit button — dims when nothing is selected
    g_killBtn = std::make_shared<ButtonNode>();
    g_killBtn->label = "Force Quit";
    g_killBtn->nodeStyle.fontSize = 11.5f;
    g_killBtn->style.setHeight(28.0f);
    g_killBtn->style.setWidth(92.0f);
    g_killBtn->nodeStyle.background = SPHXColor::RGB(50, 50, 54);
    g_killBtn->nodeStyle.foreground = SPHXColor(Mac::TextT);
    g_killBtn->nodeStyle.borderRadius = 6.0f;
    g_killBtn->onClick = []() {
        if (g_selectedPid == 0) return;
        HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, g_selectedPid);
        if (h) { TerminateProcess(h, 1); CloseHandle(h); }
        g_selectedPid = 0;
    };
    toolbar->addChild(g_killBtn);

    procCountLabel = std::make_shared<TextNode>();
    procCountLabel->text = " ";
    procCountLabel->fontSize = 11.0f;
    procCountLabel->color = Mac::TextT;
    procCountLabel->style.setWidth(108.0f);
    toolbar->addChild(procCountLabel);

    root->addChild(toolbar);
    root->addChild(std::make_shared<HSep>());

    // ── Filter bar ─────────────────────────────────────────────────────────────
    auto filterBar = FlexNode::Row();
    filterBar->style.setHeight(38.0f);
    filterBar->style.setFlexShrink(0.0f);
    filterBar->style.backgroundColor = Mac::ToolbarBg;
    filterBar->style.setPadding(8.0f, 5.0f);
    filterBar->style.setGap(10.0f);
    filterBar->style.setAlignItems(YGAlignCenter);

    auto search = std::make_shared<SearchInputNode>();
    search->style.setFlex(1.0f);
    search->style.setHeight(26.0f);
    search->style.backgroundColor = Mac::CardBg;
    search->style.borderRadius = 6.0f;
    search->onChanged = [](const std::string& text) { g_searchFilter = text; };
    filterBar->addChild(search);

    // Show Idle switch — hides processes with near-zero CPU and low memory
    auto idleSwitch = std::make_shared<SwitchNode>();
    idleSwitch->label        = "Show Idle";
    idleSwitch->isOn         = true;
    idleSwitch->fontSize     = 11.0f;
    idleSwitch->labelColor   = SPHXColor(Mac::TextS);
    idleSwitch->activeColor  = SPHXColor(Mac::Blue);
    idleSwitch->switchWidth  = 32.0f;
    idleSwitch->switchHeight = 18.0f;
    idleSwitch->onChanged    = [](bool on) { g_showIdleProcs = on; };
    filterBar->addChild(idleSwitch);

    root->addChild(filterBar);
    root->addChild(std::make_shared<HSep>());

    // ── Column headers ─────────────────────────────────────────────────────────
    auto colHeader = FlexNode::Row();
    colHeader->style.setHeight(26.0f);
    colHeader->style.setFlexShrink(0.0f);
    colHeader->style.backgroundColor = Mac::HeaderBg;
    colHeader->style.setAlignItems(YGAlignCenter);

    auto addHdr = [&](const std::string& t, float w, bool flex, int col) {
        auto c = std::make_shared<ColHeaderCell>(t, col);
        if (flex) c->style.setFlex(1.0f);
        else      c->style.setWidth(w);
        colHeader->addChild(c);
    };
    addHdr("Process Name", 0,  true,  2);
    addHdr("% CPU",        68, false, 0);
    addHdr("Threads",      62, false, 4);
    addHdr("PID",          62, false, 3);
    addHdr("Memory",       82, false, 1);

    root->addChild(colHeader);
    root->addChild(std::make_shared<HSep>());

    // ── Process list ───────────────────────────────────────────────────────────
    auto scroll = std::make_shared<ScrollAreaNode>();
    scroll->style.setFlex(1.0f);
    processListContainer = FlexNode::Column();
    scroll->setContent(processListContainer);
    root->addChild(scroll);

    // ── Splitter – drag to resize bottom panel ─────────────────────────────────
    auto splitter = std::make_shared<SplitterNode>(SplitterNode::Orientation::Horizontal);
    splitter->style.backgroundColor = SPHXColor(Mac::Sep);
    splitter->onDrag = [](float delta) {
        g_bottomHeight = std::clamp(g_bottomHeight - delta, 80.0f, 350.0f);
        if (g_bottomPanel) g_bottomPanel->style.setHeight(g_bottomHeight);
        if (g_hwnd) InvalidateRect(g_hwnd, NULL, FALSE);
    };
    root->addChild(splitter);

    // ── Bottom panel (tab-switched content) ────────────────────────────────────
    g_bottomPanel = FlexNode::Row();
    g_bottomPanel->style.setHeight(g_bottomHeight);
    g_bottomPanel->style.setFlexShrink(0.0f);
    g_bottomPanel->style.backgroundColor = Mac::BottomBg;

    // ── CPU tab content ───────────────────────────────────────────────────────
    g_cpuContent = FlexNode::Row();
    g_cpuContent->style.setFlex(1.0f);
    g_cpuContent->style.setHeightFull();
    g_cpuContent->style.setPadding(14.0f, 12.0f);
    g_cpuContent->style.setGap(14.0f);
    g_cpuContent->style.setAlignItems(YGAlignStretch);

    {
        auto cpuStats = FlexNode::Column();
        cpuStats->style.setWidth(172.0f);
        cpuStats->style.setGap(5.0f);

        auto lbl = std::make_shared<TextNode>(); lbl->text = "CPU USAGE";
        lbl->fontSize = 10.0f; lbl->color = Mac::TextT;
        cpuStats->addChild(lbl);

        // Big percentage + status badge on the same row
        auto pctRow = FlexNode::Row();
        pctRow->style.setAlignItems(YGAlignCenter);
        pctRow->style.setGap(8.0f);

        cpuPctLabel = std::make_shared<TextNode>();
        cpuPctLabel->text = "0%"; cpuPctLabel->fontSize = 28.0f; cpuPctLabel->color = Mac::TextP;
        pctRow->addChild(cpuPctLabel);

        g_cpuBadge = std::make_shared<BadgeNode>("OK");
        g_cpuBadge->nodeStyle.background = SPHXColor(Mac::Green);
        g_cpuBadge->nodeStyle.foreground = SPHXColor::white();
        g_cpuBadge->nodeStyle.fontSize = 9.0f;
        pctRow->addChild(g_cpuBadge);
        cpuStats->addChild(pctRow);

        cpuBar = std::make_shared<ProgressBar>();
        cpuBar->style.setWidthFull(); cpuBar->style.setHeight(5.0f);
        cpuBar->fillColor = Mac::Green; cpuBar->backgroundColor = Mac::CardBg; cpuBar->borderRadius = 2.5f;
        cpuStats->addChild(cpuBar);

        auto gap = FlexNode::Create(); gap->style.setFlex(1.0f);
        cpuStats->addChild(gap);
        cpuStats->addChild(MakeStatRow("User",   Mac::Blue,   userCpuLabel));
        cpuStats->addChild(MakeStatRow("System", Mac::Orange, sysCpuLabel));
        cpuStats->addChild(MakeStatRow("Idle",   Mac::TextT,  idleCpuLabel));

        g_cpuContent->addChild(cpuStats);
        g_cpuContent->addChild(std::make_shared<VSep>());

        auto graphCol = FlexNode::Column();
        graphCol->style.setFlex(1.0f); graphCol->style.setGap(5.0f);

        auto glbl = std::make_shared<TextNode>(); glbl->text = "CPU HISTORY";
        glbl->fontSize = 10.0f; glbl->color = Mac::TextT;
        graphCol->addChild(glbl);

        cpuHistGraph = std::make_shared<GraphNode>();
        cpuHistGraph->style.setFlex(1.0f); cpuHistGraph->style.setWidthFull();
        cpuHistGraph->lineColor = Mac::Green; cpuHistGraph->fillColor = SPHXColor(Mac::Green).withAlpha(uint8_t(35));
        cpuHistGraph->maxPoints = 60; cpuHistGraph->strokeWidth = 1.5f; cpuHistGraph->showGrid = true;
        graphCol->addChild(cpuHistGraph);

        g_cpuContent->addChild(graphCol);
    }

    // ── Memory tab content ────────────────────────────────────────────────────
    g_memContent = FlexNode::Row();
    g_memContent->style.setFlex(1.0f);
    g_memContent->style.setHeightFull();
    g_memContent->style.setPadding(14.0f, 12.0f);
    g_memContent->style.setGap(14.0f);
    g_memContent->style.setAlignItems(YGAlignStretch);

    {
        auto memStats = FlexNode::Column();
        memStats->style.setWidth(172.0f);
        memStats->style.setGap(5.0f);

        auto lbl = std::make_shared<TextNode>(); lbl->text = "MEMORY";
        lbl->fontSize = 10.0f; lbl->color = Mac::TextT;
        memStats->addChild(lbl);

        memUsedLabel = std::make_shared<TextNode>();
        memUsedLabel->text = "\xe2\x80\x94 GB"; memUsedLabel->fontSize = 15.0f; memUsedLabel->color = Mac::TextP;
        memStats->addChild(memUsedLabel);

        memBar = std::make_shared<ProgressBar>();
        memBar->style.setWidthFull(); memBar->style.setHeight(5.0f);
        memBar->fillColor = Mac::Orange; memBar->backgroundColor = Mac::CardBg; memBar->borderRadius = 2.5f;
        memStats->addChild(memBar);

        auto gap = FlexNode::Create(); gap->style.setFlex(1.0f);
        memStats->addChild(gap);

        std::shared_ptr<TextNode> appMem, wired, compressed;
        memStats->addChild(MakeStatRow("App",        Mac::Blue,   appMem));
        memStats->addChild(MakeStatRow("Wired",      Mac::Orange, wired));
        memStats->addChild(MakeStatRow("Compressed", Mac::TextT,  compressed));

        g_memContent->addChild(memStats);
        g_memContent->addChild(std::make_shared<VSep>());

        auto graphCol = FlexNode::Column();
        graphCol->style.setFlex(1.0f); graphCol->style.setGap(5.0f);

        auto glbl = std::make_shared<TextNode>(); glbl->text = "MEMORY PRESSURE";
        glbl->fontSize = 10.0f; glbl->color = Mac::TextT;
        graphCol->addChild(glbl);

        memHistGraph = std::make_shared<GraphNode>();
        memHistGraph->style.setFlex(1.0f); memHistGraph->style.setWidthFull();
        memHistGraph->lineColor = Mac::Orange; memHistGraph->fillColor = SPHXColor(Mac::Orange).withAlpha(uint8_t(35));
        memHistGraph->maxPoints = 60; memHistGraph->strokeWidth = 1.5f; memHistGraph->showGrid = true;
        graphCol->addChild(memHistGraph);

        g_memContent->addChild(graphCol);
    }

    // ── Wire tab switching ─────────────────────────────────────────────────────
    g_switchTab = [](int idx) {
        g_activeTab = idx;
        g_bottomPanel->removeAllChildren();
        if (idx == 0)      g_bottomPanel->addChild(g_cpuContent);
        else if (idx == 1) g_bottomPanel->addChild(g_memContent);
        if (g_hwnd) InvalidateRect(g_hwnd, NULL, FALSE);
    };

    g_bottomPanel->addChild(g_cpuContent); // default: CPU tab
    root->addChild(g_bottomPanel);

    return root;
}

// ─── Entry point ──────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Application::getInstance().init();
    Win32Window window("Activity Monitor", 1060, 800);
    window.enableMica(true);
    window.setDarkMode(true);
    window.setRoot(CreateUI());
    g_hwnd = (HWND)window.getNativeHandle();
    SetTimer(g_hwnd, 1, 1000, TimerProc);
    window.run();
    return 0;
}
