#pragma once
#include <windows.h>
#include <CommCtrl.h>
#include <string>
#include <vector>

#pragma comment(lib, "ComCtl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace MochiUI {

enum class TaskDialogIcon {
    None = 0,
    Warning,
    Error,
    Information,
    Shield
};

enum class TaskDialogButton {
    OK = TDCBF_OK_BUTTON,
    Yes = TDCBF_YES_BUTTON,
    No = TDCBF_NO_BUTTON,
    Cancel = TDCBF_CANCEL_BUTTON,
    Retry = TDCBF_RETRY_BUTTON,
    Close = TDCBF_CLOSE_BUTTON
};

enum class TaskDialogResult {
    OK = IDOK,
    Cancel = IDCANCEL,
    Retry = IDRETRY,
    Yes = IDYES,
    No = IDNO,
    Close = IDCLOSE
};

struct TaskDialogCustomButton {
    int id;
    std::wstring text;
};

class ModernTaskDialog {
public:
    ModernTaskDialog() = default;

    void setTitle(const std::wstring& title) { fTitle = title; }
    void setMainInstruction(const std::wstring& instruction) { fMainInstruction = instruction; }
    void setContent(const std::wstring& content) { fContent = content; }
    void setExpandedInfo(const std::wstring& info) { fExpandedInfo = info; }
    void setFooter(const std::wstring& footer) { fFooter = footer; }
    void setVerificationText(const std::wstring& text) { fVerificationText = text; }
    
    void setMainIcon(TaskDialogIcon icon) { fMainIcon = icon; }
    void setFooterIcon(TaskDialogIcon icon) { fFooterIcon = icon; }
    
    void setCommonButtons(int buttons) { fCommonButtons = buttons; }
    void addCustomButton(int id, const std::wstring& text) {
        fCustomButtons.push_back({id, text});
    }
    
    void setDefaultButton(int buttonId) { fDefaultButton = buttonId; }
    void setExpandedByDefault(bool expanded) { fExpandedByDefault = expanded; }
    void setUseCommandLinks(bool useLinks) { fUseCommandLinks = useLinks; }
    void setAllowDialogCancellation(bool allow) { fAllowCancellation = allow; }
    void setProgressBar(bool enable) { fShowProgressBar = enable; }
    void setMarqueeProgressBar(bool enable) { fShowMarqueeProgressBar = enable; }
    
    TaskDialogResult show(HWND hwndParent = nullptr, bool* verificationChecked = nullptr) {
        TASKDIALOGCONFIG config = {0};
        config.cbSize = sizeof(config);
        config.hwndParent = hwndParent;
        config.dwFlags = TDF_POSITION_RELATIVE_TO_WINDOW;
        
        if (fExpandedByDefault) config.dwFlags |= TDF_EXPANDED_BY_DEFAULT;
        if (fUseCommandLinks) config.dwFlags |= TDF_USE_COMMAND_LINKS;
        if (fAllowCancellation) config.dwFlags |= TDF_ALLOW_DIALOG_CANCELLATION;
        if (fShowProgressBar) config.dwFlags |= TDF_SHOW_PROGRESS_BAR;
        if (fShowMarqueeProgressBar) config.dwFlags |= TDF_SHOW_MARQUEE_PROGRESS_BAR;
        
        config.dwCommonButtons = fCommonButtons;
        
        if (!fTitle.empty()) config.pszWindowTitle = fTitle.c_str();
        if (!fMainInstruction.empty()) config.pszMainInstruction = fMainInstruction.c_str();
        if (!fContent.empty()) config.pszContent = fContent.c_str();
        if (!fExpandedInfo.empty()) config.pszExpandedInformation = fExpandedInfo.c_str();
        if (!fFooter.empty()) config.pszFooter = fFooter.c_str();
        if (!fVerificationText.empty()) config.pszVerificationText = fVerificationText.c_str();
        
        if (fMainIcon != TaskDialogIcon::None) {
            PCWSTR icon = nullptr;
            switch (fMainIcon) {
                case TaskDialogIcon::Warning: icon = TD_WARNING_ICON; break;
                case TaskDialogIcon::Error: icon = TD_ERROR_ICON; break;
                case TaskDialogIcon::Information: icon = TD_INFORMATION_ICON; break;
                case TaskDialogIcon::Shield: icon = TD_SHIELD_ICON; break;
                default: break;
            }
            config.pszMainIcon = icon;
        }
        if (fFooterIcon != TaskDialogIcon::None) {
            PCWSTR icon = nullptr;
            switch (fFooterIcon) {
                case TaskDialogIcon::Warning: icon = TD_WARNING_ICON; break;
                case TaskDialogIcon::Error: icon = TD_ERROR_ICON; break;
                case TaskDialogIcon::Information: icon = TD_INFORMATION_ICON; break;
                case TaskDialogIcon::Shield: icon = TD_SHIELD_ICON; break;
                default: break;
            }
            config.pszFooterIcon = icon;
        }
        
        if (fDefaultButton != 0) {
            config.nDefaultButton = fDefaultButton;
        }
        
        std::vector<TASKDIALOG_BUTTON> buttons;
        if (!fCustomButtons.empty()) {
            for (const auto& btn : fCustomButtons) {
                buttons.push_back({btn.id, btn.text.c_str()});
            }
            config.pButtons = buttons.data();
            config.cButtons = static_cast<UINT>(buttons.size());
        }
        
        int buttonPressed = 0;
        BOOL verificationFlag = FALSE;
        
        HRESULT hr = TaskDialogIndirect(&config, &buttonPressed, nullptr, 
                                        verificationChecked ? &verificationFlag : nullptr);
        
        if (verificationChecked) {
            *verificationChecked = (verificationFlag == TRUE);
        }
        
        if (SUCCEEDED(hr)) {
            return static_cast<TaskDialogResult>(buttonPressed);
        }
        
        return TaskDialogResult::Cancel;
    }
    
    static TaskDialogResult showMessage(HWND hwndParent, 
                                       const std::wstring& title,
                                       const std::wstring& mainInstruction,
                                       const std::wstring& content = L"",
                                       TaskDialogIcon icon = TaskDialogIcon::Information,
                                       int buttons = TDCBF_OK_BUTTON) {
        ModernTaskDialog dialog;
        dialog.setTitle(title);
        dialog.setMainInstruction(mainInstruction);
        if (!content.empty()) dialog.setContent(content);
        dialog.setMainIcon(icon);
        dialog.setCommonButtons(buttons);
        return dialog.show(hwndParent);
    }
    
    static TaskDialogResult showError(HWND hwndParent,
                                     const std::wstring& title,
                                     const std::wstring& errorMessage,
                                     const std::wstring& details = L"") {
        ModernTaskDialog dialog;
        dialog.setTitle(title);
        dialog.setMainInstruction(errorMessage);
        if (!details.empty()) dialog.setExpandedInfo(details);
        dialog.setMainIcon(TaskDialogIcon::Error);
        dialog.setCommonButtons(TDCBF_OK_BUTTON);
        return dialog.show(hwndParent);
    }
    
    static TaskDialogResult showWarning(HWND hwndParent,
                                       const std::wstring& title,
                                       const std::wstring& warningMessage,
                                       const std::wstring& content = L"") {
        ModernTaskDialog dialog;
        dialog.setTitle(title);
        dialog.setMainInstruction(warningMessage);
        if (!content.empty()) dialog.setContent(content);
        dialog.setMainIcon(TaskDialogIcon::Warning);
        dialog.setCommonButtons(TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON);
        return dialog.show(hwndParent);
    }
    
    static bool confirm(HWND hwndParent,
                       const std::wstring& title,
                       const std::wstring& question,
                       const std::wstring& content = L"") {
        ModernTaskDialog dialog;
        dialog.setTitle(title);
        dialog.setMainInstruction(question);
        if (!content.empty()) dialog.setContent(content);
        dialog.setMainIcon(TaskDialogIcon::None);
        dialog.setCommonButtons(TDCBF_YES_BUTTON | TDCBF_NO_BUTTON);
        auto result = dialog.show(hwndParent);
        return result == TaskDialogResult::Yes;
    }

private:
    std::wstring fTitle;
    std::wstring fMainInstruction;
    std::wstring fContent;
    std::wstring fExpandedInfo;
    std::wstring fFooter;
    std::wstring fVerificationText;
    
    TaskDialogIcon fMainIcon = TaskDialogIcon::None;
    TaskDialogIcon fFooterIcon = TaskDialogIcon::None;
    
    int fCommonButtons = TDCBF_OK_BUTTON;
    int fDefaultButton = 0;
    
    std::vector<TaskDialogCustomButton> fCustomButtons;
    
    bool fExpandedByDefault = false;
    bool fUseCommandLinks = false;
    bool fAllowCancellation = true;
    bool fShowProgressBar = false;
    bool fShowMarqueeProgressBar = false;
};

} // namespace MochiUI
