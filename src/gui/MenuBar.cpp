#include <gui/MenuBar.hpp>
#include <SPHXGraphicComponents.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <windows.h>
#include <map>

namespace SphereUI {

// --- Win32 Backend Implementation ---
class Win32MenuBar : public IMenuBar {
    HMENU hMenuBar;
    std::map<int, std::function<void()>> actions;

public:
    Win32MenuBar() {
        hMenuBar = CreateMenu();
    }

    void addMenu(const std::string& label, const std::vector<MenuItem>& items) override {
        HMENU hMenu = CreatePopupMenu();
        for (const auto& item : items) {
            AppendMenuA(hMenu, MF_STRING, item.id, item.label.c_str());
            if (item.action) {
                actions[item.id] = item.action;
            }
        }
        AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hMenu, label.c_str());
    }

    void attach(void* windowHandle) override {
        SetMenu((HWND)windowHandle, hMenuBar);
    }

    FlexNode::Ptr getLayoutNode() override { return nullptr; }
};

// Custom menu item node with modern rendering
class MenuItemNode : public FlexNode {
public:
    std::string text;
    SkColor textColor = Theme::TextPrimary;
    SkColor hoverColor = Theme::Accent;
    int fontSize = 11;
    std::vector<MenuItem> menuItems;

    MenuItemNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }

    Size measure(Size available) override {
        SkRect textBounds;
        float textWidth = FontManager::getInstance().measureText(text, (float)fontSize, &textBounds);
        return { textWidth + 16.0f, 24.0f };
    }

    void draw(SkCanvas* canvas) override {
        if (!canvas) return;

        if (isHovered) {
            SkPaint bgPaint;
            bgPaint.setAntiAlias(true);
            bgPaint.setColor(hoverColor);
            bgPaint.setAlphaf(0.2f);
            canvas->drawRect(frame, bgPaint);
        }

        SkPaint textPaint;
        textPaint.setColor(textColor);
        textPaint.setAntiAlias(true);

        SkFontMetrics metrics;
        FontManager::getInstance().getFontMetrics((float)fontSize, &metrics);

        float textWidth = FontManager::getInstance().measureText(text, (float)fontSize);
        float textX = std::round(frame.left() + (frame.width() - textWidth) / 2.0f);
        float textY = std::round(frame.centerY() - (metrics.fAscent + metrics.fDescent) / 2.0f);

        FontManager::getInstance().drawText(canvas, text, textX, textY, (float)fontSize, textPaint);
    }

    bool onMouseDown(float x, float y) override {
        if (hitTest(x, y)) {
            showPopupMenu();
            return true;
        }
        return false;
    }

    void showPopupMenu() {
        if (menuItems.empty()) return;

        HMENU hMenu = CreatePopupMenu();
        for (const auto& item : menuItems) {
            AppendMenuA(hMenu, MF_STRING, item.id, item.label.c_str());
        }

        POINT pt = { (long)frame.left(), (long)frame.bottom() };
        ClientToScreen((HWND)GetActiveWindow(), &pt);

        int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY,
                                pt.x, pt.y, 0, GetActiveWindow(), nullptr);

        if (cmd > 0) {
            for (const auto& item : menuItems) {
                if (item.id == cmd && item.action) {
                    item.action();
                    break;
                }
            }
        }
        DestroyMenu(hMenu);
    }
};

// --- Skia Backend Implementation ---
class SkiaMenuBar : public IMenuBar {
    FlexNode::Ptr rootNode;

public:
    SkiaMenuBar() {
        rootNode = FlexNode::Row();
        rootNode->style.setHeight(28);
        rootNode->style.backgroundColor = Theme::MenuBar;
        rootNode->style.setAlignItems(YGAlignCenter);
        rootNode->style.setPadding(2);
    }

    void addMenu(const std::string& label, const std::vector<MenuItem>& items) override {
        auto menuBtn = std::make_shared<MenuItemNode>();
        menuBtn->text = label;
        menuBtn->style.setHeight(24);
        menuBtn->textColor = Theme::TextPrimary;
        menuBtn->hoverColor = Theme::Accent;
        menuBtn->fontSize = 12;
        menuBtn->menuItems = items;
        rootNode->addChild(menuBtn);
    }

    void attach(void* windowHandle) override { }

    FlexNode::Ptr getLayoutNode() override {
        return rootNode;
    }
};

std::unique_ptr<IMenuBar> MenuBarFactory::Create(MenuBackend backend) {
    if (backend == MenuBackend::Win32) return std::make_unique<Win32MenuBar>();
    return std::make_unique<SkiaMenuBar>();
}

} // namespace SphereUI
