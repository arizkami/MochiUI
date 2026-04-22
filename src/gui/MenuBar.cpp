#include <include/gui/MenuBar.hpp>
#include <include/gui/Components.hpp>
#include <include/gui/Theme.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <windows.h>
#include <map>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFontMgr.h>

namespace MochiUI {

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
    bool isHovered = false;
    std::vector<MenuItem> menuItems;
    
    Size measure(Size available) override {
        float textWidth = 0;
        float textHeight = fontSize;
        
        SkRect textBounds;
        FontManager::getInstance().measureText(text, (float)fontSize, &textBounds);
        textWidth = textBounds.width();
        textHeight = textBounds.height();
        
        float totalWidth = textWidth + 2 * style.padding;
        float totalHeight = textHeight + 2 * style.padding;
        
        return { totalWidth, totalHeight };
    }
    
    void draw(SkCanvas* canvas) override {
        if (!canvas) return;
        
        SkPaint bgPaint;
        if (isHovered) {
            bgPaint.setColor(hoverColor);
            bgPaint.setAlphaf(0.15f);
        } else {
            bgPaint.setColor(SK_ColorTRANSPARENT);
        }
        
        SkRRect rrect = SkRRect::MakeRectXY(frame, style.borderRadius, style.borderRadius);
        canvas->drawRRect(rrect, bgPaint);
        
        SkPaint textPaint;
        textPaint.setColor(textColor);
        textPaint.setAntiAlias(true);
        
        SkRect textBounds;
        FontManager::getInstance().measureText(text, (float)fontSize, &textBounds);

        float textX = frame.left() + (frame.width() - textBounds.width()) / 2;
        float textY = frame.centerY() - textBounds.centerY();

        FontManager::getInstance().drawText(canvas, text, textX, textY, (float)fontSize, textPaint);

        FlexNode::draw(canvas);    }
    
    bool onMouseMove(float x, float y) override {
        bool wasHovered = isHovered;
        isHovered = frame.contains(x, y);
        return wasHovered != isHovered || FlexNode::onMouseMove(x, y);
    }
    
    bool onMouseDown(float x, float y) override {
        if (frame.contains(x, y)) {
            showPopupMenu();
            return true;
        }
        return FlexNode::onMouseDown(x, y);
    }
    
    void showPopupMenu() {
        if (menuItems.empty()) return;
        
        HMENU hMenu = CreatePopupMenu();
        for (const auto& item : menuItems) {
            AppendMenuA(hMenu, MF_STRING, item.id, item.label.c_str());
        }
        
        POINT pt;
        GetCursorPos(&pt);
        
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
        rootNode->style.height = 22;
        rootNode->style.heightMode = SizingMode::Fixed;
        rootNode->style.widthMode = SizingMode::Flex;
        rootNode->style.backgroundColor = Theme::MenuBar;
        rootNode->style.padding = 2;
        rootNode->style.gap = 1;
    }

    void addMenu(const std::string& label, const std::vector<MenuItem>& items) override {
        auto menuBtn = std::make_shared<MenuItemNode>();
        menuBtn->text = label;
        menuBtn->style.widthMode = SizingMode::Hug;
        menuBtn->style.heightMode = SizingMode::Flex;
        menuBtn->style.padding = 6;
        menuBtn->style.borderRadius = 3;
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

} // namespace MochiUI
