#include "screen_utils.h"
#include <string> // For stoull
#include <math.h>

using namespace std;

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
    info->autoHideTaskbar = 0;
    
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

inline size_t get_virtual_screens_buffer_size() {
    size_t buff_size = (sizeof(PhysicalScreen) * MAX_SCREENS) + sizeof(int) + sizeof(int) + sizeof(int32_t) + (sizeof(uint8_t) * 4) + sizeof(uint32_t);
    return buff_size;
}

char* getGMSBuffAddress(char* _GMSBuffPtrStr) {
    /*
        @description    Converts a GMS buffer address string to a usable pointer in C++.
        @params         {char*} _GMSBuffPtrStr - The ptr to a GMS buffer as a string.
        @return         {char*} The pointer to the buffer. Now functions like memcpy will work.
    */
    size_t GMSBuffLongPointer = stoull(_GMSBuffPtrStr, NULL, 16);//Gets the ptr string into and int64_t.
    return (char*)GMSBuffLongPointer;//Casts the int64_t pointer to char* and returns it so the buffer can be now operated in C++.
}

// Write a value of type T into buf, advance buf by sizeof(T)
template<typename T>
inline char* GMSWrite(char* buf, const T& val) {
    // Test for current or impending buf ovverflow and return nullptr
    if((buf == nullptr) || ((buf + sizeof(T)) > (buf + get_virtual_screens_buffer_size()))) {
        return nullptr;
    }
    std::memcpy(buf, &val, sizeof(T));
    return buf + sizeof(T);
}

// Specialize bool so it always writes 1 byte (0 or 1)
inline char* GMSWrite(char* buf, bool val) {
    uint8_t b = val ? 1 : 0;
    // Test for current or impending buf ovverflow and return nullptr
    if((buf == nullptr) || ((buf + sizeof(b)) > (buf + get_virtual_screens_buffer_size()))) {
        return nullptr;
    }
    std::memcpy(buf, &b, sizeof(b));
    return buf + sizeof(b);
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

double ext_get_virtual_screens(char* inbuf) {
    PhysicalScreen screenArray[MAX_SCREENS];
    ScreenArrayInfo info;

    // Initialize the struct to pass to the library function
    info.screen = screenArray;
    info.count = 0;
    info.maxCount = MAX_SCREENS;
    info.more = false;

//    char *buf;
    char* buf = getGMSBuffAddress(inbuf);//Interpret the string address form GMS so it can be managed by C++
    
//    buf = getGMSBuffAddress(inbuf);
    
    // Call the function from the DLL
    if(__internal_get_virtual_screens(&info)) {
        buf = GMSWrite(buf, info.count);
        buf = GMSWrite(buf, info.maxCount);
        buf = GMSWrite(buf, info.autoHideTaskbar);
        buf = GMSWrite(buf, info.more);
        buf = GMSWrite(buf, info.versionMajor);
        buf = GMSWrite(buf, info.versionMinor);
        buf = GMSWrite(buf, info.versionBuild);
        for(int i = 0; i < info.count; i++) {
            buf = GMSWrite(buf, info.screen[i].infoLevel);
            buf = GMSWrite(buf, info.screen[i].refreshRate);
            buf = GMSWrite(buf, info.screen[i].isPrimary);

            buf = GMSWrite(buf, info.screen[i].pixelRect.width);
            buf = GMSWrite(buf, info.screen[i].pixelRect.height);

            buf = GMSWrite(buf, info.screen[i].virtualRect.left);
            buf = GMSWrite(buf, info.screen[i].virtualRect.top);
            buf = GMSWrite(buf, info.screen[i].virtualRect.width);
            buf = GMSWrite(buf, info.screen[i].virtualRect.height);

            buf = GMSWrite(buf, info.screen[i].taskbarRect.left);
            buf = GMSWrite(buf, info.screen[i].taskbarRect.top);
            buf = GMSWrite(buf, info.screen[i].taskbarRect.width);
            buf = GMSWrite(buf, info.screen[i].taskbarRect.height);

            buf = GMSWrite(buf, info.screen[i].macmenuRect.left);
            buf = GMSWrite(buf, info.screen[i].macmenuRect.top);
            buf = GMSWrite(buf, info.screen[i].macmenuRect.width);
            buf = GMSWrite(buf, info.screen[i].macmenuRect.height);

            buf = GMSWrite(buf, info.screen[i].physSize.width);
            buf = GMSWrite(buf, info.screen[i].physSize.height);
            buf = GMSWrite(buf, info.screen[i].physSize.diagonal);
        }
        if (info.count < MAX_SCREENS) {
            PhysicalScreen empty = {};
            for(int i = info.count; i < MAX_SCREENS; i++) {
                buf = GMSWrite(buf, empty);
            }
        }
            buf = GMSWrite(buf, info.fourcc);
        // buf will be a nullptr if overflow occurred
        if(buf != nullptr) {
        // buf is fine, return 1
            return 1;
        }
    }
    
    // buf is bad, return 1
    return 0;
}

double ext_get_virtual_screens_buffer_size() {
    return get_virtual_screens_buffer_size();
}

double ext_get_screens_data_size() {
    return sizeof(PhysicalScreen);
}