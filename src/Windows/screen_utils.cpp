#include "screen_utils.h"
#include <string> // For stoull
#include <math.h>

// Helper functions

static VirtualRect RectToScreen(LPRECT lprcMonitor) {
    VirtualRect rect;
    rect.left = lprcMonitor->left;
    rect.top = lprcMonitor->top;
    rect.width = abs(lprcMonitor->left - lprcMonitor->right);
    rect.height = abs(lprcMonitor->bottom - lprcMonitor->top);
    return rect;
}

// This is the internal callback function for EnumDisplayMonitors.
// It is also marked 'static' and kept private to this file.
static BOOL CALLBACK MonitorEnum(
    HMONITOR hMonitor, // Monitor Handle
    HDC hdc, // Unused
    LPRECT lprcMonitor, // Scaled rect of this screen
    LPARAM pData // For passing data around
    ) {
    ScreenArrayInfo* info = reinterpret_cast<ScreenArrayInfo*>(pData);

    info->screen[info->count].virtualRect = RectToScreen(lprcMonitor);
    info->screen[info->count].taskbarRect = { 0,0,0,0 };
    info->screen[info->count].macmenuRect = { 0,0,0,0 };
    info->screen[info->count].infoLevel = 0;
    
    MONITORINFOEX monitorInfo; // Used to get Primary + Display Name
  
    monitorInfo.cbSize = sizeof(MONITORINFOEX);
    if (GetMonitorInfo(hMonitor, &monitorInfo)) {
        info->screen[info->count].infoLevel = 1;
        info->screen[info->count].isPrimary = (monitorInfo.dwFlags & MONITORINFOF_PRIMARY);

        // --- Get Native/Physical Pixel Resolution using EnumDisplaySettingsEx ---
        // This gives the true resolution of the monitor's current display mode.
        DEVMODE devMode;
        devMode.dmSize = sizeof(DEVMODE);
        devMode.dmDriverExtra = 0; // Must be 0 for EnumDisplaySettingsEx

        if (EnumDisplaySettingsEx(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS,
                                  &devMode, 0)) {
            info->screen[info->count].infoLevel         = 2;
            info->screen[info->count].pixelRect.width   = devMode.dmPelsWidth;
            info->screen[info->count].pixelRect.height  = devMode.dmPelsHeight;
            info->screen[info->count].refreshRate       = devMode.dmDisplayFrequency;

            // --- Get physical dimensions (mm) using GetDeviceCaps ---
            HDC hdc = CreateDC(monitorInfo.szDevice, nullptr, nullptr, nullptr);
            if (hdc) {
                info->screen[info->count].infoLevel = 3;
                int32_t pwidth = GetDeviceCaps(hdc, HORZSIZE); // Physical width in mm
                info->screen[info->count].physSize.width = pwidth;
                int32_t pheight = GetDeviceCaps(hdc, VERTSIZE); // Physical height in mm
                info->screen[info->count].physSize.height = pheight;
                info->screen[info->count].physSize.diagonal = lround(sqrt((pheight * pheight) + (pwidth * pwidth)));
                
                DeleteDC(hdc); // Always release the DC
            } else {
                info->screen[info->count].physSize = { 0, 0, 0 };
            }


        } else {
            info->screen[info->count].pixelRect = { 0,0 };
            info->screen[info->count].refreshRate = 0;
        }
        
        // fprintf(stderr, "deviceName = %s\n", deviceName.c_str());
    } else {
        info->screen[info->count].isPrimary = false;
    }
    
    

    info->count++;

    if (info->count == info->maxCount) {
        info->more = true;
        return false;
    }
    
    return true;
}

static char* getGMSBuffAddress(char* _GMSBuffPtrStr) {
  size_t GMSBuffLongPointer = std::stoull(_GMSBuffPtrStr, NULL, 16);
    return (char*)GMSBuffLongPointer;
}

// --- Implementation of Exported Functions ---

BOOL __internal_get_virtual_screens(ScreenArrayInfo* info) {
  return EnumDisplayMonitors(
    NULL,
    NULL,
    &MonitorEnum,
    reinterpret_cast<LPARAM>(info)
  );
}

double ext_get_virtual_screens(char* buf) {
    PhysicalScreen screenArray[MAX_SCREENS];
    ScreenArrayInfo info;

    // Initialize the struct to pass to the library function
    info.screen = screenArray;
    info.count = 0;
    info.maxCount = MAX_SCREENS;
    info.more = false;

    // Call the function from the DLL
    if(__internal_get_virtual_screens(&info)) {
    }
    
    return 0;
}

double ext_get_virtual_screens_buffer_size() {
    size_t buff_size = (sizeof(PhysicalScreen) * MAX_SCREENS) + sizeof(int) + sizeof(int) + sizeof(boolean) + (sizeof(byte) * 3);
    return buff_size;
}
