#pragma once
#include <windows.h>
#include <shobjidl.h>
#include <string>
#include <vector>

namespace AureliaUI {

struct FileFilter {
    std::wstring name;
    std::wstring spec;
};

class FileDialog {
public:
    static std::wstring OpenFile(HWND parent = nullptr,
                               const std::wstring& title = L"Open File",
                               const std::vector<FileFilter>& filters = {}) {
        std::wstring result;
        IFileOpenDialog* pFileOpen;

        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                                     IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr)) {
            pFileOpen->SetTitle(title.c_str());

            if (!filters.empty()) {
                std::vector<COMDLG_FILTERSPEC> spec;
                for (const auto& f : filters) {
                    spec.push_back({ f.name.c_str(), f.spec.c_str() });
                }
                pFileOpen->SetFileTypes((UINT)spec.size(), spec.data());
            }

            hr = pFileOpen->Show(parent);

            if (SUCCEEDED(hr)) {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (SUCCEEDED(hr)) {
                        result = pszFilePath;
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        return result;
    }

    static std::wstring SaveFile(HWND parent = nullptr,
                               const std::wstring& title = L"Save File",
                               const std::vector<FileFilter>& filters = {},
                               const std::wstring& defaultExtension = L"") {
        std::wstring result;
        IFileSaveDialog* pFileSave;

        HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
                                     IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

        if (SUCCEEDED(hr)) {
            pFileSave->SetTitle(title.c_str());

            if (!defaultExtension.empty()) {
                pFileSave->SetDefaultExtension(defaultExtension.c_str());
            }

            if (!filters.empty()) {
                std::vector<COMDLG_FILTERSPEC> spec;
                for (const auto& f : filters) {
                    spec.push_back({ f.name.c_str(), f.spec.c_str() });
                }
                pFileSave->SetFileTypes((UINT)spec.size(), spec.data());
            }

            hr = pFileSave->Show(parent);

            if (SUCCEEDED(hr)) {
                IShellItem* pItem;
                hr = pFileSave->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (SUCCEEDED(hr)) {
                        result = pszFilePath;
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileSave->Release();
        }
        return result;
    }

    static std::wstring SelectFolder(HWND parent = nullptr,
                                   const std::wstring& title = L"Select Folder") {
        std::wstring result;
        IFileOpenDialog* pFileOpen;

        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                                     IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr)) {
            DWORD dwOptions;
            if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions))) {
                pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
            }

            pFileOpen->SetTitle(title.c_str());

            hr = pFileOpen->Show(parent);

            if (SUCCEEDED(hr)) {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (SUCCEEDED(hr)) {
                        result = pszFilePath;
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        return result;
    }
};

} // namespace AureliaUI
