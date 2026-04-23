#pragma once
#include <windows.h>
#include <string>

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace MochiUI {

enum class ConfirmationIcon {
    None = 0,
    Warning = MB_ICONWARNING,
    Error = MB_ICONERROR,
    Information = MB_ICONINFORMATION,
    Shield = MB_ICONWARNING
};

enum class ConfirmationResult {
    OK = IDOK,
    Cancel = IDCANCEL,
    Retry = IDRETRY,
    Yes = IDYES,
    No = IDNO,
    Close = IDCLOSE
};

class ConfirmationDialog {
public:
    ConfirmationDialog() = default;

    void setTitle(const std::wstring& title) { fTitle = title; }
    void setMainInstruction(const std::wstring& instruction) { fMainInstruction = instruction; }
    void setContent(const std::wstring& content) { fContent = content; }
    void setExpandedInfo(const std::wstring& info) { fExpandedInfo = info; }
    void setFooter(const std::wstring& footer) { fFooter = footer; }
    
    void setMainIcon(ConfirmationIcon icon) { fMainIcon = icon; }
    void setCommonButtons(UINT buttons) { fCommonButtons = buttons; }
    
    ConfirmationResult show(HWND hwndParent = nullptr) {
        std::wstring fullMessage = fMainInstruction;
        if (!fMainInstruction.empty() && !fContent.empty()) {
            fullMessage += L"\n\n";
        }
        fullMessage += fContent;

        if (!fExpandedInfo.empty()) {
            fullMessage += L"\n\n" + fExpandedInfo;
        }
        
        if (!fFooter.empty()) {
            fullMessage += L"\n\n--- " + fFooter;
        }

        UINT type = fCommonButtons;
        if (fMainIcon != ConfirmationIcon::None) {
            type |= static_cast<UINT>(fMainIcon);
        }

        int result = MessageBoxW(hwndParent, fullMessage.c_str(), fTitle.empty() ? L"Message" : fTitle.c_str(), type);
        return static_cast<ConfirmationResult>(result);
    }
    
    static ConfirmationResult showMessage(HWND hwndParent, 
                                       const std::wstring& title,
                                       const std::wstring& mainInstruction,
                                       const std::wstring& content = L"",
                                       ConfirmationIcon icon = ConfirmationIcon::Information,
                                       UINT buttons = MB_OK) {
        ConfirmationDialog dialog;
        dialog.setTitle(title);
        dialog.setMainInstruction(mainInstruction);
        if (!content.empty()) dialog.setContent(content);
        dialog.setMainIcon(icon);
        dialog.setCommonButtons(buttons);
        return dialog.show(hwndParent);
    }
    
    static ConfirmationResult showError(HWND hwndParent,
                                     const std::wstring& title,
                                     const std::wstring& errorMessage,
                                     const std::wstring& details = L"") {
        ConfirmationDialog dialog;
        dialog.setTitle(title);
        dialog.setMainInstruction(errorMessage);
        if (!details.empty()) dialog.setExpandedInfo(details);
        dialog.setMainIcon(ConfirmationIcon::Error);
        dialog.setCommonButtons(MB_OK);
        return dialog.show(hwndParent);
    }
    
    static ConfirmationResult showWarning(HWND hwndParent,
                                       const std::wstring& title,
                                       const std::wstring& warningMessage,
                                       const std::wstring& content = L"") {
        ConfirmationDialog dialog;
        dialog.setTitle(title);
        dialog.setMainInstruction(warningMessage);
        if (!content.empty()) dialog.setContent(content);
        dialog.setMainIcon(ConfirmationIcon::Warning);
        dialog.setCommonButtons(MB_OKCANCEL);
        return dialog.show(hwndParent);
    }
    
    static bool confirm(HWND hwndParent,
                       const std::wstring& title,
                       const std::wstring& question,
                       const std::wstring& content = L"") {
        ConfirmationDialog dialog;
        dialog.setTitle(title);
        dialog.setMainInstruction(question);
        if (!content.empty()) dialog.setContent(content);
        dialog.setMainIcon(ConfirmationIcon::None);
        dialog.setCommonButtons(MB_YESNO);
        auto result = dialog.show(hwndParent);
        return result == ConfirmationResult::Yes;
    }

private:
    std::wstring fTitle;
    std::wstring fMainInstruction;
    std::wstring fContent;
    std::wstring fExpandedInfo;
    std::wstring fFooter;
    
    ConfirmationIcon fMainIcon = ConfirmationIcon::None;
    UINT fCommonButtons = MB_OK;
};

} // namespace MochiUI
