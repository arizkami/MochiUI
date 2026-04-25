#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace MochiUI {

struct Shortcut {
    uint32_t key;
    bool ctrl;
    bool shift;
    bool alt;
    std::function<void()> action;
};

class ShortcutsManager {
public:
    static ShortcutsManager& getInstance() {
        static ShortcutsManager instance;
        return instance;
    }

    void registerShortcut(uint32_t key, bool ctrl, bool shift, bool alt, std::function<void()> action) {
        shortcuts.push_back({key, ctrl, shift, alt, action});
    }

    bool handleKey(uint32_t key, bool ctrl, bool shift, bool alt) {
        for (const auto& s : shortcuts) {
            if (s.key == key && s.ctrl == ctrl && s.shift == shift && s.alt == alt) {
                s.action();
                return true;
            }
        }
        return false;
    }

private:
    ShortcutsManager() = default;
    std::vector<Shortcut> shortcuts;
};

} // namespace MochiUI
