#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <commctrl.h>
#include <shlobj.h>

#pragma comment(lib, "comctl32.lib")

// Constants
#define IDC_PROCESS_LIST 101
#define IDC_DLL_PATH_EDIT 102
#define IDC_BROWSE_BUTTON 103
#define IDC_INJECT_BUTTON 104
#define IDC_FILTER_EDIT 105
#define IDC_REFRESH_BUTTON 106

// Global variables
HWND g_hProcessList;
HWND g_hDllPathEdit;
HWND g_hFilterEdit;
std::vector<DWORD> g_processIds;

// Function prototypes
bool InjectDLL(DWORD pid, const std::wstring& dllPath);
std::vector<std::wstring> GetProcessList(const std::wstring& filter = L"");
void RefreshProcessList(HWND hList, const std::wstring& filter = L"");
std::wstring BrowseForDLL(HWND hOwner);

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        // Create filter edit box
        CreateWindowW(L"Static", L"Filter:", WS_VISIBLE | WS_CHILD, 10, 10, 50, 20, hwnd, NULL, NULL, NULL);
        g_hFilterEdit = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 60, 10, 300, 20, hwnd, (HMENU)IDC_FILTER_EDIT, NULL, NULL);
        
        // Create refresh button
        CreateWindowW(L"Button", L"Refresh", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 370, 10, 80, 20, hwnd, (HMENU)IDC_REFRESH_BUTTON, NULL, NULL);

        // Create process list
        g_hProcessList = CreateWindowW(L"ListBox", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY | WS_VSCROLL, 10, 40, 440, 200, hwnd, (HMENU)IDC_PROCESS_LIST, NULL, NULL);
        
        // Create DLL path controls
        CreateWindowW(L"Static", L"DLL Path:", WS_VISIBLE | WS_CHILD, 10, 250, 50, 20, hwnd, NULL, NULL, NULL);
        g_hDllPathEdit = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 60, 250, 350, 20, hwnd, (HMENU)IDC_DLL_PATH_EDIT, NULL, NULL);
        CreateWindowW(L"Button", L"Browse", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 420, 250, 80, 20, hwnd, (HMENU)IDC_BROWSE_BUTTON, NULL, NULL);
        
        // Create inject button
        CreateWindowW(L"Button", L"Inject DLL", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 280, 440, 30, hwnd, (HMENU)IDC_INJECT_BUTTON, NULL, NULL);

        // Populate process list
        RefreshProcessList(g_hProcessList);
        return 0;
    }
    
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_REFRESH_BUTTON:
        {
            wchar_t filterText[256];
            GetWindowTextW(g_hFilterEdit, filterText, 256);
            RefreshProcessList(g_hProcessList, std::wstring(filterText));
            break;
        }
        
        case IDC_BROWSE_BUTTON:
        {
            std::wstring dllPath = BrowseForDLL(hwnd);
            if (!dllPath.empty())
            {
                SetWindowTextW(g_hDllPathEdit, dllPath.c_str());
            }
            break;
        }
        
        case IDC_INJECT_BUTTON:
        {
            int selectedIndex = (int)SendMessageW(g_hProcessList, LB_GETCURSEL, 0, 0);
            if (selectedIndex == LB_ERR)
            {
                MessageBoxW(hwnd, L"Please select a process first", L"Error", MB_ICONERROR);
                break;
            }

            wchar_t dllPath[MAX_PATH];
            GetWindowTextW(g_hDllPathEdit, dllPath, MAX_PATH);
            if (wcslen(dllPath) == 0)
            {
                MessageBoxW(hwnd, L"Please select a DLL first", L"Error", MB_ICONERROR);
                break;
            }

            if (selectedIndex >= 0 && selectedIndex < (int)g_processIds.size())
            {
                DWORD pid = g_processIds[selectedIndex];
                if (InjectDLL(pid, dllPath))
                {
                    MessageBoxW(hwnd, L"DLL injected successfully!", L"Success", MB_ICONINFORMATION);
                }
                else
                {
                    MessageBoxW(hwnd, L"Failed to inject DLL", L"Error", MB_ICONERROR);
                }
            }
            break;
        }
        }
        return 0;
    }
    
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    }
    
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Initialize common controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    // Register window class
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszClassName = L"DLLInjectorClass";
    
    if (!RegisterClassW(&wc))
    {
        MessageBoxW(NULL, L"Window registration failed", L"Error", MB_ICONERROR);
        return 1;
    }

    // Create window
    HWND hwnd = CreateWindowW(
        L"DLLInjectorClass",
        L"DLL Injector",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 400,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd)
    {
        MessageBoxW(NULL, L"Window creation failed", L"Error", MB_ICONERROR);
        return 1;
    }

    // Show window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}

bool InjectDLL(DWORD pid, const std::wstring& dllPath)
{
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc || hProc == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    size_t size = (dllPath.size() + 1) * sizeof(wchar_t);
    LPVOID remoteMemory = VirtualAllocEx(hProc, NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteMemory)
    {
        CloseHandle(hProc);
        return false;
    }

    if (!WriteProcessMemory(hProc, remoteMemory, dllPath.c_str(), size, NULL))
    {
        VirtualFreeEx(hProc, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"),
        remoteMemory, 0, NULL);

    if (!hThread)
    {
        VirtualFreeEx(hProc, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);

    DWORD exitCode;
    GetExitCodeThread(hThread, &exitCode);

    VirtualFreeEx(hProc, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProc);

    return (exitCode != 0);
}

std::vector<std::wstring> GetProcessList(const std::wstring& filter)
{
    std::vector<std::wstring> processes;
    g_processIds.clear();

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return processes;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe))
    {
        do
        {
            std::wstring processName = pe.szExeFile;
            if (filter.empty() || 
                (processName.find(filter) != std::wstring::npos))
            {
                processes.push_back(processName);
                g_processIds.push_back(pe.th32ProcessID);
            }
        } while (Process32NextW(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return processes;
}

void RefreshProcessList(HWND hList, const std::wstring& filter)
{
    SendMessageW(hList, LB_RESETCONTENT, 0, 0);
    std::vector<std::wstring> processes = GetProcessList(filter);
    
    for (const auto& process : processes)
    {
        SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)process.c_str());
    }
}

std::wstring BrowseForDLL(HWND hOwner)
{
    OPENFILENAMEW ofn = {0};
    wchar_t szFile[MAX_PATH] = {0};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hOwner;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"DLL Files\0*.dll\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn))
    {
        return std::wstring(ofn.lpstrFile);
    }
    return L"";
}
