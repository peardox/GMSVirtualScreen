#include <windows.h>
#include <shellapi.h>       // for SHAppBarMessage
#include <vector>
#include <iostream>

struct Monitor {
    HMONITOR hMon;
    RECT     rcMonitor;      // monitor bounds
    RECT     rcTaskbar;      // taskbar window rect (if any)
    bool     hasTaskbar;     // did we find a taskbar window?
    bool     taskbarAutoHide;// is auto-hide on (i.e. taskbar “hidden”)?
};

// 1) collect monitor bounds
BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC, LPRECT, LPARAM pData) {
    MONITORINFO mi = { sizeof(mi) };
    if (!GetMonitorInfo(hMon, &mi))
        return TRUE;
    Monitor m;
    m.hMon = hMon;
    m.rcMonitor = mi.rcMonitor;
    SetRect(&m.rcTaskbar, 0, 0, 0, 0);
    m.hasTaskbar = false;
    m.taskbarAutoHide = false;
    reinterpret_cast<std::vector<Monitor>*>(pData)->push_back(m);
    return TRUE;
}

// 2) find each Shell_TrayWnd / Shell_SecondaryTrayWnd
BOOL CALLBACK EnumTaskbarProc(HWND hwnd, LPARAM pData) {
    if (!IsWindowVisible(hwnd))
        return TRUE;
    char cls[32] = { 0 };
    if (GetClassNameA(hwnd, cls, sizeof(cls)) == 0)
        return TRUE;
    if (strcmp(cls, "Shell_TrayWnd") != 0 &&
        strcmp(cls, "Shell_SecondaryTrayWnd") != 0)
        return TRUE;

    HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);
    if (!hMon)
        return TRUE;

    RECT tr;
    if (!GetWindowRect(hwnd, &tr))
        return TRUE;

    auto* mons = reinterpret_cast<std::vector<Monitor>*>(pData);
    for (auto& m : *mons) {
        if (m.hMon == hMon) {
            m.rcTaskbar = tr;
            m.hasTaskbar = true;
            break;
        }
    }
    return TRUE;
}

int main() {
    std::vector<Monitor> monitors;

    if (!EnumDisplayMonitors(nullptr, nullptr,
        MonitorEnumProc,
        reinterpret_cast<LPARAM>(&monitors)))
    {
        std::cerr << "EnumDisplayMonitors failed\n";
        return 1;
    }

    EnumWindows(EnumTaskbarProc,
        reinterpret_cast<LPARAM>(&monitors));

    // 3) query global auto-hide flag
    APPBARDATA abd = { sizeof(abd) };
    DWORD state = SHAppBarMessage(ABM_GETSTATE, &abd);
    bool autoHide = (state & ABS_AUTOHIDE) != 0;

    // 4) assign auto-hide to each monitor that actually has a taskbar
    for (auto& m : monitors) {
        if (m.hasTaskbar)
            m.taskbarAutoHide = autoHide;
    }

    // 5) print
    for (size_t i = 0; i < monitors.size(); ++i) {
        const auto& m = monitors[i];
        int w = m.rcMonitor.right - m.rcMonitor.left;
        int h = m.rcMonitor.bottom - m.rcMonitor.top;
        std::cout << "Monitor " << (i + 1)
            << ": " << w << "x" << h
            << " @ (" << m.rcMonitor.left
            << "," << m.rcMonitor.top << ")";

        if (m.hasTaskbar) {
            std::cout << ", Taskbar RECT=("
                << m.rcTaskbar.left << ","
                << m.rcTaskbar.top << ")-("
                << m.rcTaskbar.right << ","
                << m.rcTaskbar.bottom << ")"
                << ", Hidden(auto-hide)="
                << (m.taskbarAutoHide ? "Yes" : "No");
        }
        else {
            std::cout << ", Taskbar: <none>";
        }
        std::cout << '\n';
    }

    return 0;
}