#include <codecvt>   // For std::codecvt_utf8_utf16 (deprecated but common)
#include <cmath>     // For sqrt and pow
#include <iomanip>   // For std::fixed and std::setprecision
#include <iostream>
#include <locale>    // For std::locale
#include <memory>    // For std::unique_ptr
#include <string>
#include <vector>
#include <windows.h> // For WinAPI functions

// Helper function to convert TCHAR[] to std::string
// This handles both ANSI and Unicode builds correctly.
std::string TCHAR_to_string(const TCHAR *tstr) {
  if (!tstr)
    return "";

#ifdef UNICODE
  std::wstring ws(tstr);
  int bufferSize = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0,
                                      nullptr, nullptr);
  if (bufferSize == 0) {
    return "";
  }
  std::string result(bufferSize - 1, '\0');
  WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &result[0], bufferSize,
                      nullptr, nullptr);
  return result;

#else
  return std::string(tstr);
#endif
}

// Structure to hold monitor information
struct MonitorInfo {
  bool isPrimary;                // <<< ADDED: True if this is the primary monitor
  std::string deviceName;        // e.g., \\.\DISPLAY1
  std::string deviceDescription; // e.g., Generic PnP Monitor, Dell U2719DC

  // --- Native/Physical Pixel Resolution (True resolution of the display panel)
  // ---
  int nativePixelWidth;
  int nativePixelHeight;
  int refreshRate;

  // --- Virtual Desktop Coordinates (Monitor's position and size within the
  // virtual screen) --- This is how the monitor is laid out in the overall
  // Windows desktop space.
  int virtualDesktopX;      // Top-left X coordinate on the virtual desktop
  int virtualDesktopY;      // Top-left Y coordinate on the virtual desktop
  int virtualDesktopWidth;  // Width within the virtual desktop coordinate system
  int virtualDesktopHeight; // Height within the virtual desktop coordinate
                            // system

  // --- Physical Size in Millimeters (Actual physical dimensions of the panel)
  // ---
  int physicalWidthMM;
  int physicalHeightMM;
};

// Callback function for EnumDisplayMonitors
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor,
                              LPRECT lprcMonitor, LPARAM dwData) {
  std::vector<MonitorInfo> *monitors =
      reinterpret_cast<std::vector<MonitorInfo> *>(dwData);

  MONITORINFOEX monitorInfo;
  monitorInfo.cbSize = sizeof(MONITORINFOEX);

  if (GetMonitorInfo(hMonitor, &monitorInfo)) {
    MonitorInfo currentMonitor;

    // <<< MODIFIED: Check the dwFlags member for the primary monitor flag
    currentMonitor.isPrimary = (monitorInfo.dwFlags & MONITORINFOF_PRIMARY);

    currentMonitor.deviceName = TCHAR_to_string(monitorInfo.szDevice);

    // --- Retrieve Device Description using EnumDisplayDevices ---
    DISPLAY_DEVICE dd;
    dd.cb = sizeof(DISPLAY_DEVICE);
    // The monitorInfo.szDevice (e.g., "\\.\DISPLAY1") refers to the display
    // adapter. We need to enumerate the *monitor* devices associated with this
    // adapter.
    for (DWORD devNum = 0; EnumDisplayDevices(monitorInfo.szDevice, devNum, &dd, 0);
         ++devNum) {
      // Check if it's a monitor device (and not the adapter itself or other
      // devices)
      if (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP &&
          !(dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)) {
        // If the device is attached to desktop and not the primary device,
        // it's likely a monitor connected to the adapter.
        // More robust checking might involve correlating device ID.
        // For simplicity, we'll take the first non-primary monitor attached
        // or just the first attached monitor if it's the primary.
        currentMonitor.deviceDescription = TCHAR_to_string(dd.DeviceString);
        break; // Found the description for this device
      } else if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
        // If it's the primary device (often corresponds to the main monitor)
        currentMonitor.deviceDescription = TCHAR_to_string(dd.DeviceString);
        break;
      }
    }
    // If not found above, try again by enumerating without the
    // DISPLAY_DEVICE_ATTACHED_TO_DESKTOP flag or just directly from the first
    // child device.
    if (currentMonitor.deviceDescription.empty()) {
      DISPLAY_DEVICE monitorDevice;
      monitorDevice.cb = sizeof(DISPLAY_DEVICE);
      if (EnumDisplayDevices(monitorInfo.szDevice, 0, &monitorDevice,
                             EDD_GET_DEVICE_INTERFACE_NAME)) {
        currentMonitor.deviceDescription =
            TCHAR_to_string(monitorDevice.DeviceString);
      } else {
        currentMonitor.deviceDescription = "Unknown Monitor";
      }
    }

    // --- Get Native/Physical Pixel Resolution using EnumDisplaySettingsEx ---
    // This gives the true resolution of the monitor's current display mode.
    DEVMODE devMode;
    devMode.dmSize = sizeof(DEVMODE);
    devMode.dmDriverExtra = 0; // Must be 0 for EnumDisplaySettingsEx

    if (EnumDisplaySettingsEx(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS,
                              &devMode, 0)) {
      currentMonitor.nativePixelWidth = devMode.dmPelsWidth;
      currentMonitor.nativePixelHeight = devMode.dmPelsHeight;
      currentMonitor.refreshRate = devMode.dmDisplayFrequency;
    } else {
      std::cerr << "Warning: Could not get native pixel resolution for monitor '"
                << currentMonitor.deviceName
                << "'. Using virtual desktop size instead.\n";
      // Fallback to virtual desktop size if native resolution can't be
      // retrieved
      currentMonitor.nativePixelWidth =
          monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
      currentMonitor.nativePixelHeight =
          monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.left;
      currentMonitor.refreshRate = 0;
    }

    // --- Virtual Desktop Coordinates for this Monitor ---
    // rcMonitor provides the monitor's bounding box relative to the
    // virtual screen's origin (0,0, usually top-left of primary monitor).
    currentMonitor.virtualDesktopX = monitorInfo.rcMonitor.left;
    currentMonitor.virtualDesktopY = monitorInfo.rcMonitor.top;
    currentMonitor.virtualDesktopWidth =
        monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
    currentMonitor.virtualDesktopHeight =
        monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

    // --- Get physical dimensions (mm) using GetDeviceCaps ---
    HDC hdc = CreateDC(monitorInfo.szDevice, nullptr, nullptr, nullptr);
    if (hdc) {
      currentMonitor.physicalWidthMM =
          GetDeviceCaps(hdc, HORZSIZE); // Physical width in mm
      currentMonitor.physicalHeightMM =
          GetDeviceCaps(hdc, VERTSIZE); // Physical height in mm
      DeleteDC(hdc);                    // Always release the DC
    } else {
      std::cerr << "Warning: Could not create DC for monitor '"
                << currentMonitor.deviceName << "' to get physical size.\n";
      currentMonitor.physicalWidthMM = 0;
      currentMonitor.physicalHeightMM = 0;
    }

    monitors->push_back(currentMonitor);
  } else {
    std::cerr << "Warning: GetMonitorInfo failed for a monitor.\n";
  }
  return TRUE; // Continue enumeration to the next monitor
}

int main() {
  std::vector<MonitorInfo> monitors;

  EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc,
                      reinterpret_cast<LPARAM>(&monitors));

  if (monitors.empty()) {
    std::cout << "No monitors found.\n";
    return 1;
  }

  std::cout << "Detected Windows Monitors (Per Screen):\n";
  for (const auto &mon : monitors) {
    std::cout << "----------------------------------------\n";
    // <<< MODIFIED: Print the primary monitor status
    std::cout << "Is Primary Monitor:           " << (mon.isPrimary ? "Yes" : "No")
              << "\n";
    std::cout << "Device Name:                  " << mon.deviceName << "\n";
    std::cout << "Device Description:           " << mon.deviceDescription
              << "\n";
    std::cout << "Native Pixel Resolution:      " << mon.nativePixelWidth
              << " x " << mon.nativePixelHeight << " pixels\n";
    std::cout << "Refresh Rate:                 " << mon.refreshRate << " Hz\n";
    std::cout << "Virtual Desktop Position:     (" << mon.virtualDesktopX
              << ", " << mon.virtualDesktopY << ")\n";
    std::cout << "Virtual Desktop Size:         " << mon.virtualDesktopWidth
              << " x " << mon.virtualDesktopHeight << " pixels\n";
    std::cout << "Physical Size (Actual):       " << mon.physicalWidthMM
              << " mm x " << mon.physicalHeightMM << " mm\n";

    if (mon.physicalWidthMM > 0 && mon.physicalHeightMM > 0) {
      double widthInches = mon.physicalWidthMM / 25.4;
      double heightInches = mon.physicalHeightMM / 25.4;
      double diagonalInches = sqrt(pow(widthInches, 2) + pow(heightInches, 2));
      std::cout << "Approx. Diagonal Size:        " << std::fixed
                << std::setprecision(1) << diagonalInches << " inches\n";
    } else {
      std::cout
          << "Approx. Diagonal Size:        Not available (physical size "
             "unknown)\n";
    }
  }
  std::cout << "----------------------------------------\n";

  // Optional: Also display the total virtual desktop dimensions, as it's often
  // useful context.
  int virtualScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  int virtualScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  int virtualScreenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
  int virtualScreenY = GetSystemMetrics(SM_YVIRTUALSCREEN);

  std::cout << "\n----------------------------------------\n";
  std::cout << "Overall Virtual Desktop Information:\n";
  std::cout << "  Top-Left Corner: (" << virtualScreenX << ", "
            << virtualScreenY << ")\n";
  std::cout << "  Total Size:      " << virtualScreenWidth << " x "
            << virtualScreenHeight << " pixels\n";
  std::cout << "----------------------------------------\n";

  return 0;
}