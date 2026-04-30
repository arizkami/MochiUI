#include <MCKApplication.hpp>
#include <MCKGraphicInterface.hpp>
#include <MCKGraphicComponents.hpp>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <map>

using namespace MochiUI;

struct ProcessInfo {
    uint32_t pid;
    std::string name;
    float cpuUsage;
    uint64_t memoryUsage;
};

class ProcessProvider {
public:
    struct ProcState {
        ULONGLONG lastTime;
        ULONGLONG lastProcTime;
    };

    std::vector<ProcessInfo> getProcesses() {
        std::vector<ProcessInfo> list;
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap == INVALID_HANDLE_VALUE) return list;

        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);

        if (Process32FirstW(hSnap, &pe)) {
            do {
                ProcessInfo info;
                info.pid = pe.th32ProcessID;
                char name[MAX_PATH];
                WideCharToMultiByte(CP_UTF8, 0, pe.szExeFile, -1, name, MAX_PATH, NULL, NULL);
                info.name = name;

                HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.pid);
                if (hProc) {
                    PROCESS_MEMORY_COUNTERS pmc;
                    if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc))) {
                        info.memoryUsage = pmc.WorkingSetSize;
                    }
                    
                    FILETIME createTime, exitTime, kernelTime, userTime;
                    if (GetProcessTimes(hProc, &createTime, &exitTime, &kernelTime, &userTime)) {
                        ULARGE_INTEGER k, u;
                        k.LowPart = kernelTime.dwLowDateTime; k.HighPart = kernelTime.dwHighDateTime;
                        u.LowPart = userTime.dwLowDateTime; u.HighPart = userTime.dwHighDateTime;
                        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
                        auto procTime = k.QuadPart + u.QuadPart;
                        if (states.count(info.pid)) {
                            auto deltaProc = procTime - states[info.pid].lastProcTime;
                            info.cpuUsage = (float)deltaProc / 100000.0f; 
                            if (info.cpuUsage > 100.0f) info.cpuUsage = 0.1f; 
                        } else info.cpuUsage = 0.0f;
                        states[info.pid] = { (ULONGLONG)now, (ULONGLONG)procTime };
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

std::shared_ptr<GraphNode> ramGraph;
std::shared_ptr<FlexNode> processListContainer;
std::shared_ptr<TextNode> ramText;

FlexNode::Ptr CreateProcessRow(const ProcessInfo& info) {
    auto row = FlexNode::Row();
    row->style.setHeight(30);
    row->style.setPadding(4);
    row->style.setGap(10);
    row->style.setAlignItems(YGAlignCenter);

    auto createCell = [](std::string txt, float w, bool flex = false) {
        auto t = std::make_shared<TextNode>();
        t->text = txt;
        t->fontSize = 12;
        t->color = Theme::TextPrimary;
        if (flex) t->style.setFlex(1.0f);
        else t->style.setWidth(w);
        return t;
    };

    row->addChild(createCell(info.name, 200, true));
    row->addChild(createCell(std::to_string(info.pid), 60));
    char cpuBuf[16]; sprintf(cpuBuf, "%.1f%%", info.cpuUsage);
    row->addChild(createCell(cpuBuf, 60));
    char memBuf[32];
    if (info.memoryUsage > 1024 * 1024 * 1024) sprintf(memBuf, "%.1f GB", (float)info.memoryUsage / (1024 * 1024 * 1024));
    else sprintf(memBuf, "%.1f MB", (float)info.memoryUsage / (1024 * 1024));
    row->addChild(createCell(memBuf, 90));
    return row;
}

void CALLBACK TimerProc(HWND hwnd, UINT, UINT_PTR, DWORD) {
    static ProcessProvider provider;
    auto procs = provider.getProcesses();
    std::sort(procs.begin(), procs.end(), [](const ProcessInfo& a, const ProcessInfo& b) { return a.cpuUsage > b.cpuUsage; });

    if (processListContainer) {
        processListContainer->children.clear();
        // Clear Yoga children too!
        YGNodeRemoveAllChildren(processListContainer->getYGNode());
        for (size_t i = 0; i < std::min(procs.size(), (size_t)40); ++i) {
            processListContainer->addChild(CreateProcessRow(procs[i]));
        }
    }

    MEMORYSTATUSEX mem; mem.dwLength = sizeof(mem); GlobalMemoryStatusEx(&mem);
    float ramUsage = (float)mem.dwMemoryLoad / 100.0f;
    if (ramGraph) ramGraph->addValue(ramUsage);
    if (ramText) {
        char buf[32]; sprintf(buf, "%.1f GB", (float)(mem.ullTotalPhys - mem.ullAvailPhys) / (1024*1024*1024));
        ramText->text = buf;
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

FlexNode::Ptr CreateUI() {
    auto root = FlexNode::Column();
    root->style.backgroundColor = Theme::Background;

    auto toolbar = FlexNode::Row();
    toolbar->style.setHeight(60);
    toolbar->style.backgroundColor = Theme::Sidebar;
    toolbar->style.setPadding(10);
    toolbar->style.setGap(10);

    auto createTab = [](std::string label) {
        auto b = std::make_shared<ButtonNode>();
        b->label = label;
        b->style.setWidth(100);
        return b;
    };
    toolbar->addChild(createTab("CPU"));
    toolbar->addChild(createTab("Memory"));
    toolbar->addChild(createTab("Disk"));
    toolbar->addChild(createTab("Network"));
    root->addChild(toolbar);

    auto header = FlexNode::Row();
    header->style.setHeight(32);
    header->style.backgroundColor = SkColorSetRGB(50, 50, 50);
    header->style.setPadding(5);
    header->style.setGap(10);
    
    auto createH = [](std::string t, float w, bool flex = false) {
        auto n = std::make_shared<TextNode>();
        n->text = t; n->fontSize = 11; n->color = Theme::TextSecondary;
        if (flex) n->style.setFlex(1.0f);
        else n->style.setWidth(w);
        return n;
    };
    header->addChild(createH("Process Name", 200, true));
    header->addChild(createH("PID", 60));
    header->addChild(createH("% CPU", 60));
    header->addChild(createH("Memory", 90));
    root->addChild(header);

    auto scroll = std::make_shared<ScrollAreaNode>();
    scroll->style.setFlex(1.0f);
    processListContainer = FlexNode::Column();
    processListContainer->style.setFlex(1.0f); // Yoga will handle content height
    scroll->setContent(processListContainer);
    root->addChild(scroll);

    auto bottom = FlexNode::Row();
    bottom->style.setHeight(150);
    bottom->style.backgroundColor = Theme::Sidebar;
    bottom->style.setPadding(20);
    bottom->style.setGap(30);

    auto ramBox = FlexNode::Column();
    ramBox->style.setWidth(200);
    auto ramTitle = std::make_shared<TextNode>();
    ramTitle->text = "Memory Used:"; ramTitle->fontSize = 13; ramTitle->color = Theme::TextSecondary;
    ramBox->addChild(ramTitle);
    ramText = std::make_shared<TextNode>();
    ramText->text = "0.0 GB"; ramText->fontSize = 18;
    ramBox->addChild(ramText);
    bottom->addChild(ramBox);

    ramGraph = std::make_shared<GraphNode>();
    ramGraph->style.setFlex(1.0f);
    ramGraph->lineColor = SkColorSetRGB(0, 255, 120);
    bottom->addChild(ramGraph);

    root->addChild(bottom);
    return root;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Application::getInstance().init();
    Win32Window window("Activity Monitor", 1000, 750);
    window.enableMica(true);
    window.setDarkMode(true);
    window.setRoot(CreateUI());
    SetTimer((HWND)window.getNativeHandle(), 1, 1000, TimerProc);
    window.run();
    return 0;
}
