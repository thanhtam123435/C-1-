
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <iomanip>
#include <sstream>

using namespace std::chrono_literals;

std::atomic<bool> g_running{true};

std::string now_timestamp() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::ostringstream oss;
    oss << std::setfill('0')
        << st.wYear << "-" << std::setw(2) << st.wMonth << "-" << std::setw(2) << st.wDay
        << " " << std::setw(2) << st.wHour << ":" << std::setw(2) << st.wMinute << ":" << std::setw(2) << st.wSecond;
    return oss.str();
}

std::string ws_to_string(const std::wstring &ws) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), NULL, 0, NULL, NULL);
    std::string s(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &s[0], size_needed, NULL, NULL);
    return s;
}

std::string get_process_name_by_pid(DWORD pid) {
    std::string name = "<unknown>";
    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProc) {
        HMODULE hMod;
        DWORD cbNeeded;
        if (EnumProcessModules(hProc, &hMod, sizeof(hMod), &cbNeeded)) {
            wchar_t exeName[MAX_PATH];
            if (GetModuleBaseNameW(hProc, hMod, exeName, MAX_PATH)) {
                name = ws_to_string(std::wstring(exeName));
            }
        }
        CloseHandle(hProc);
    }
    return name;
}

void active_window_monitor(int poll_ms = 800) {
    HWND lastWnd = nullptr;
    DWORD lastPid = 0;
    std::string lastTitle;
    while (g_running) {
        HWND fg = GetForegroundWindow();
        if (fg != lastWnd) {
            wchar_t title[512] = {0};
            GetWindowTextW(fg, title, _countof(title));
            DWORD pid = 0;
            GetWindowThreadProcessId(fg, &pid);
            std::string procName = get_process_name_by_pid(pid);
            std::string titleStr = ws_to_string(std::wstring(title));
            std::cout << "[" << now_timestamp() << "] ActiveWindow -> PID: " << pid
                      << ", Process: " << procName << ", Title: \"" << titleStr << "\"\n";
            lastWnd = fg;
            lastPid = pid;
            lastTitle = titleStr;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_ms));
    }
}

std::unordered_map<DWORD, std::string> snapshot_processes() {
    std::unordered_map<DWORD, std::string> map;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return map;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    if (Process32First(hSnap, &pe)) {
        do {
            map[pe.th32ProcessID] = ws_to_string(pe.szExeFile);
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    return map;
}

void process_monitor(int interval_ms = 2000) {
    auto prev = snapshot_processes();
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        auto curr = snapshot_processes();

        // Detect new processes
        for (const auto &p : curr) {
            if (prev.find(p.first) == prev.end()) {
                std::cout << "[" << now_timestamp() << "] Process started -> PID: " << p.first
                          << ", Name: " << p.second << "\n";
            }
        }
        // Detect terminated processes
        for (const auto &p : prev) {
            if (curr.find(p.first) == curr.end()) {
                std::cout << "[" << now_timestamp() << "] Process ended   -> PID: " << p.first
                          << ", Name: " << p.second << "\n";
            }
        }
        prev.swap(curr);
    }
}

void directory_monitor(const std::wstring &path) {
    HANDLE hDir = CreateFileW(
        path.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );

    if (hDir == INVALID_HANDLE_VALUE) {
        std::wcerr << L"Cannot open directory: " << path << L". Try running as administrator or check path.\n";
        return;
    }

    BYTE buffer[4096];
    DWORD bytesReturned;
    while (g_running) {
        if (ReadDirectoryChangesW(
                hDir,
                &buffer,
                sizeof(buffer),
                TRUE, // monitor subdirs
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE,
                &bytesReturned,
                NULL,
                NULL)) {

            DWORD offset = 0;
            while (offset < bytesReturned) {
                FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)(buffer + offset);
                std::wstring name(fni->FileName, fni->FileName + (fni->FileNameLength / sizeof(WCHAR)));
                std::string nameUtf8 = ws_to_string(name);
                std::string action;
                switch (fni->Action) {
                    case FILE_ACTION_ADDED: action = "Added"; break;
                    case FILE_ACTION_REMOVED: action = "Removed"; break;
                    case FILE_ACTION_MODIFIED: action = "Modified"; break;
                    case FILE_ACTION_RENAMED_OLD_NAME: action = "Renamed(from)"; break;
                    case FILE_ACTION_RENAMED_NEW_NAME: action = "Renamed(to)"; break;
                    default: action = "Unknown"; break;
                }
                std::cout << "[" << now_timestamp() << "] FileEvent -> " << action << ": " << nameUtf8 << "\n";

                if (fni->NextEntryOffset == 0) break;
                offset += fni->NextEntryOffset;
            }
        } else {
            std::cerr << "[" << now_timestamp() << "] ReadDirectoryChangesW failed or stopped.\n";
            break;
        }
    }
    CloseHandle(hDir);
}

int main() {
    std::cout << "=== Simple Local Monitor (Windows) ===\n";
    std::cout << "Press Q then Enter to quit.\n";

    // Choose directory to monitor; default to user's Desktop
    wchar_t userProfile[MAX_PATH];
    std::wstring desktopPath;
    if (GetEnvironmentVariableW(L"USERPROFILE", userProfile, MAX_PATH)) {
        desktopPath = std::wstring(userProfile) + L"\\Desktop";
    } else {
        desktopPath = L"C:\\";
    }
    std::wcout << L"Monitoring directory (including subfolders): " << desktopPath << L"\n";

    std::thread t_active(active_window_monitor, 800);
    std::thread t_proc(process_monitor, 2000);
    std::thread t_dir(directory_monitor, desktopPath);

    while (g_running) {
        std::string cmd;
        if (!std::getline(std::cin, cmd)) break;
        if (!cmd.empty()) {
            if (cmd == "q" || cmd == "Q" || cmd == "quit" || cmd == "exit") {
                g_running = false;
            } else {
                std::cout << "Unknown command. Type Q then Enter to quit.\n";
            }
        }
    }

    if (t_active.joinable()) t_active.join();
    if (t_proc.joinable()) t_proc.join();
    if (t_dir.joinable()) t_dir.join();

    std::cout << "Monitor stopped.\n";
    return 0;
}
