#include <windows.h>
#include <vector>
#include <iostream>
#include <string>

using namespace std;

// Function to get display config info for a specific monitor
bool GetDisplayConfigForMonitor(HMONITOR hMonitor, 
                               DISPLAYCONFIG_PATH_INFO& matchedPath,
                               DISPLAYCONFIG_TARGET_DEVICE_NAME& targetName,
                               DISPLAYCONFIG_ADAPTER_NAME& adapterName)
{
    // Get monitor info for the HMONITOR
    MONITORINFOEX monitorInfo = {};
    monitorInfo.cbSize = sizeof(MONITORINFOEX);
    if (!GetMonitorInfo(hMonitor, &monitorInfo))
    {
        return false;
    }

    vector<DISPLAYCONFIG_PATH_INFO> paths;
    vector<DISPLAYCONFIG_MODE_INFO> modes;
    UINT32 flags = QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE;
    LONG result = ERROR_SUCCESS;

    do
    {
        // Determine how many path and mode structures to allocate
        UINT32 pathCount, modeCount;
        result = GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);

        if (result != ERROR_SUCCESS)
        {
            return false;
        }

        // Allocate the path and mode arrays
        paths.resize(pathCount);
        modes.resize(modeCount);

        // Get all active paths and their modes
        result = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);

        // The function may have returned fewer paths/modes than estimated
        paths.resize(pathCount);
        modes.resize(modeCount);

    } while (result == ERROR_INSUFFICIENT_BUFFER);

    if (result != ERROR_SUCCESS)
    {
        return false;
    }

    // Find the path that corresponds to our HMONITOR
    for (auto& path : paths)
    {
        // Get the source mode info to compare position
        if (path.sourceInfo.modeInfoIdx < modes.size())
        {
            auto& sourceMode = modes[path.sourceInfo.modeInfoIdx].sourceMode;
            
            // Compare the position and size with monitor rect
            RECT monitorRect = monitorInfo.rcMonitor;
            if (sourceMode.position.x == monitorRect.left &&
                sourceMode.position.y == monitorRect.top &&
                sourceMode.width == (monitorRect.right - monitorRect.left) &&
                sourceMode.height == (monitorRect.bottom - monitorRect.top))
            {
                matchedPath = path;

                // Get target device name
                targetName = {};
                targetName.header.adapterId = path.targetInfo.adapterId;
                targetName.header.id = path.targetInfo.id;
                targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
                targetName.header.size = sizeof(targetName);
                result = DisplayConfigGetDeviceInfo(&targetName.header);

                if (result != ERROR_SUCCESS)
                {
                    return false;
                }

                // Get adapter device name
                adapterName = {};
                adapterName.header.adapterId = path.targetInfo.adapterId;
                adapterName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME;
                adapterName.header.size = sizeof(adapterName);

                result = DisplayConfigGetDeviceInfo(&adapterName.header);

                if (result != ERROR_SUCCESS)
                {
                    return false;
                }

                return true;
            }
        }
    }

    return false;
}

// Callback function for EnumDisplayMonitors
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    DISPLAYCONFIG_PATH_INFO matchedPath;
    DISPLAYCONFIG_TARGET_DEVICE_NAME targetName;
    DISPLAYCONFIG_ADAPTER_NAME adapterName;

    if (GetDisplayConfigForMonitor(hMonitor, matchedPath, targetName, adapterName))
    {
        wcout
            << L"Monitor with name "
            << (targetName.flags.friendlyNameFromEdid ? targetName.monitorFriendlyDeviceName : L"Unknown")
            << L" is connected to adapter "
            << adapterName.adapterDevicePath
            << L" on target "
            << matchedPath.targetInfo.id
            << L"\n";
    }
    else
    {
        wcout << L"Could not get display config info for monitor\n";
    }

    return TRUE; // Continue enumeration
}

int main()
{
    // Enumerate all monitors and get their display config info
    if (!EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0))
    {
        wcout << L"Failed to enumerate monitors\n";
        return 1;
    }

    return 0;
}