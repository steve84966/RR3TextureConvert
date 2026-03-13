#pragma once
#include"framework.h"

inline void process_print(wchar_t* _a, wchar_t* _b);
inline void ListAllFiles(wchar_t* basicpath, void(*process)(wchar_t*, wchar_t*));

inline void ListAllFiles(wchar_t* basicpath, void(*process)(wchar_t*, wchar_t*)) {
    std::wstring path(basicpath);
    // 确保路径以反斜杠结尾
    if (path.back() != L'\\') {
        path += L'\\';
    }

    std::vector<std::wstring> dirs;
    dirs.push_back(path);

    while (!dirs.empty()) {
        std::wstring currentDir = dirs.back();
        dirs.pop_back();
        WIN32_FIND_DATAW findData;
        std::wstring searchPath = currentDir + L"*";
        HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                // 跳过当前目录和父目录
                if (wcscmp(findData.cFileName, L".") == 0 ||
                    wcscmp(findData.cFileName, L"..") == 0) {
                    continue;
                }

                std::wstring fullPath = currentDir + findData.cFileName;
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    // 如果是目录，加入待处理列表
                    dirs.push_back(fullPath + L"\\");
                }
                else {
                    // 如果是文件，计算相对路径并处理
                    std::wstring relativePath = fullPath.substr(path.length());
                    process(const_cast<wchar_t*>(path.c_str()), const_cast<wchar_t*>(relativePath.c_str()));
                }

            } while (FindNextFileW(hFind, &findData) != 0);
            FindClose(hFind);
        }
    }
}

inline void process_print(wchar_t* _a, wchar_t* _b) {
    //std::wcout << _a << L'\t' << _b << L'\n';
    //wprintf(L"%s\t%s\n", _a, _b);
    wchar_t buffer[512];
    int a = swprintf_s(buffer, 512, L"%s\t%s\n", _a, _b);
    DWORD _;
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), buffer, a, &_, NULL);
}