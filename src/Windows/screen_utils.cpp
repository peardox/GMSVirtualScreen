#include "screen_utils.h"
#include <string> // For stoull
#include <math.h>
#include <stdio.h>
#include <shellscalingapi.h>
#include <utility>
#include <vector>
#include <iostream>

#pragma comment(lib, "shcore.lib")

using namespace std;

// Helper functions

static GMSRect RectToGMSRect(LPRECT lprcMonitor) {
    GMSRect rect;
    rect.left = lprcMonitor->left;
    rect.top = lprcMonitor->top;
    rect.right = lprcMonitor->right;
    rect.bottom = lprcMonitor->bottom;
    return rect;
}

static GMSRect RectToGMSRect(RECT rcMonitor) {
    GMSRect rect;
    rect.left = rcMonitor.left;
    rect.top = rcMonitor.top;
    rect.right = rcMonitor.right;
    rect.bottom = rcMonitor.bottom;
    return rect;
}

// Function to get monitor friendly name for a specific HMONITORINFOEX
std::string GetMonitorFriendlyName(MONITORINFOEX monitorInfo, bool& okflag)
{
	okflag = false;
	
    vector<DISPLAYCONFIG_PATH_INFO> paths;
    vector<DISPLAYCONFIG_MODE_INFO> modes;
    UINT32 flags = QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE;
    LONG result = ERROR_SUCCESS;

    do
    {
        UINT32 pathCount, modeCount;
        result = GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);

        if (result != ERROR_SUCCESS)
        {
            return "Unknown Monitor";
        }

        paths.resize(pathCount);
        modes.resize(modeCount);

        result = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);

        paths.resize(pathCount);
        modes.resize(modeCount);

    } while (result == ERROR_INSUFFICIENT_BUFFER);

    if (result != ERROR_SUCCESS)
    {
        return "Unknown Monitor";
    }

    // Convert the narrow device name to wide string for comparison
    int wideLength = MultiByteToWideChar(CP_ACP, 0, monitorInfo.szDevice, -1, nullptr, 0);
    if (wideLength <= 0)
    {
        return "Unknown Monitor";
    }
    
    wstring monitorDeviceName(wideLength - 1, L'\0'); // -1 to exclude null terminator
    MultiByteToWideChar(CP_ACP, 0, monitorInfo.szDevice, -1, &monitorDeviceName[0], wideLength);

    // Find the matching path by comparing device names
    for (auto& path : paths)
    {
        // Get the source device name for this path
        DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
        sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
        sourceName.header.size = sizeof(sourceName);
        sourceName.header.adapterId = path.sourceInfo.adapterId;
        sourceName.header.id = path.sourceInfo.id;

        result = DisplayConfigGetDeviceInfo(&sourceName.header);
        if (result == ERROR_SUCCESS)
        {
            // Compare the source device name with our monitor's device name
            if (monitorDeviceName == sourceName.viewGdiDeviceName)
            {
                // Found matching path, now get the target (monitor) friendly name
                DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
                targetName.header.adapterId = path.targetInfo.adapterId;
                targetName.header.id = path.targetInfo.id;
                targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
                targetName.header.size = sizeof(targetName);
                
                result = DisplayConfigGetDeviceInfo(&targetName.header);
                if (result != ERROR_SUCCESS)
                {
                    return "Unknown Monitor";
                }

                // Prefer EDID friendly name, fallback to monitor friendly device name
                const wchar_t* nameToUse = nullptr;
                if (targetName.flags.friendlyNameFromEdid && wcslen(targetName.monitorFriendlyDeviceName) > 0)
                {
                    nameToUse = targetName.monitorFriendlyDeviceName;
                }
                else if (wcslen(targetName.monitorFriendlyDeviceName) > 0)
                {
                    nameToUse = targetName.monitorFriendlyDeviceName;
                }

                if (nameToUse == nullptr)
                {
					okflag = true;
                    return "Internal Display";
                }

                // Convert wide string to UTF-8 string
                int utf8Length = WideCharToMultiByte(
                    CP_UTF8, 0, nameToUse, -1, nullptr, 0, nullptr, nullptr
                );

                if (utf8Length <= 0)
                {
                    return "Unknown Monitor";
                }

                string friendlyName(utf8Length - 1, '\0'); // -1 to exclude null terminator
                WideCharToMultiByte(
                    CP_UTF8, 0, nameToUse, -1, 
                    &friendlyName[0], utf8Length, nullptr, nullptr
                );

			    okflag = true;
                return friendlyName;
            }
        }
    }

    return "Unknown Monitor";
}

static BOOL CALLBACK MonitorEnum(
    HMONITOR hMonitor, // Monitor Handle
    HDC hdc, // Unused
    LPRECT lprcMonitor, // Scaled rect of this screen
    LPARAM pData // For passing data around
    ) {
    ScreenInfo* info = reinterpret_cast<ScreenInfo*>(pData);
/*
    if (info->count < (info->pageNum * MAX_SCREENS)) {
        info->count++;
        return true;
    }
*/
    info->screen[info->count].virtualRect = RectToGMSRect(lprcMonitor);
    info->screen[info->count].workingRect = { 0,0,0,0 };
    info->screen[info->count].errorCode = 0;

    info->autoHideTaskbar = 0;
    
    MONITORINFOEX monitorInfo; // Used to get Primary + Display Name
  
    monitorInfo.cbSize = sizeof(MONITORINFOEX);
    if (GetMonitorInfo(hMonitor, &monitorInfo)) {
		bool nameok;
 		std:string mn = GetMonitorFriendlyName(monitorInfo, nameok);
		if(!nameok) {
            info->screen[info->count].errorCode |= 8;
		}
		
		std::strncpy(info->screen[info->count].name, mn.c_str(), MONITOR_NAME_BUFFER_SIZE - 1);
		info->screen[info->count].name[MONITOR_NAME_BUFFER_SIZE - 1] = '\0';
		
        info->screen[info->count].isPrimary = (monitorInfo.dwFlags & MONITORINFOF_PRIMARY);
		info->screen[info->count].workingRect = RectToGMSRect(monitorInfo.rcWork);

        // --- Get Native/Physical Pixel Resolution using EnumDisplaySettingsEx ---
        // This gives the true resolution of the monitor's current display mode.
        DEVMODE devMode;
        devMode.dmSize = sizeof(DEVMODE);
        devMode.dmDriverExtra = 0; // Must be 0 for EnumDisplaySettingsEx

        if (EnumDisplaySettingsEx(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS,
                                  &devMode, 0)) {
            info->screen[info->count].pixelBox.width   = devMode.dmPelsWidth;
            info->screen[info->count].pixelBox.height  = devMode.dmPelsHeight;
            info->screen[info->count].refreshRate       = devMode.dmDisplayFrequency;

            // --- Get physical dimensions (mm) using GetDeviceCaps ---
            HDC hdc = CreateDC(monitorInfo.szDevice, nullptr, nullptr, nullptr);
            if (hdc) {
                int32_t pwidth = GetDeviceCaps(hdc, HORZSIZE); // Physical width in mm
                info->screen[info->count].physSize.width = pwidth;
                int32_t pheight = GetDeviceCaps(hdc, VERTSIZE); // Physical height in mm
                info->screen[info->count].physSize.height = pheight;
                info->screen[info->count].physSize.diagonal = lround(sqrt((pheight * pheight) + (pwidth * pwidth)));
                
                DeleteDC(hdc); // Always release the DC
            } else {
                info->screen[info->count].errorCode |= 4;
                info->screen[info->count].physSize = { 0, 0, 0 };
            }


        } else {
            info->screen[info->count].errorCode |= 2;
            info->screen[info->count].pixelBox = { 0,0 };
            info->screen[info->count].refreshRate = 0;
        }
        
    } else {
        info->screen[info->count].errorCode |= 1;
        info->screen[info->count].isPrimary = false;
    }
    
    info->count++;

    if (info->count == info->maxCount) {
        info->more = true;
        return false;
    }
    
    return true;
}

static inline size_t rezol_get_buffer_size(int32_t which) {
    size_t buff_size;
    
    switch(which) {
        case SCREENINFOHEADER:
            buff_size = (5 * sizeof(int32_t)) + (4 * sizeof(uint8_t));
            break;
        case SCREENINFO:
            buff_size = (5 * sizeof(int32_t)) + (4 * sizeof(uint8_t)) + (sizeof(PhysicalScreen) * MAX_SCREENS) + sizeof(uint32_t);
            break;
        case PHYSICALSCREEN:
            buff_size = sizeof(PhysicalScreen);
            break;
        case WINDOWCHROME:
            buff_size = sizeof(WindowChrome);
            break;
        default:
            buff_size = 0;
            break;
    }
    
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
    if((buf == nullptr) || ((buf + sizeof(T)) > (buf + rezol_get_buffer_size(SCREENINFO)))) {
        return nullptr;
    }
    std::memcpy(buf, &val, sizeof(T));
    return buf + sizeof(T);
}

// Specialize bool so it always writes 1 byte (0 or 1)
inline char* GMSWrite(char* buf, bool val) {
    uint8_t b = val ? 1 : 0;
    // Test for current or impending buf ovverflow and return nullptr
    if((buf == nullptr) || ((buf + sizeof(b)) > (buf + rezol_get_buffer_size(SCREENINFO)))) {
        return nullptr;
    }
    std::memcpy(buf, &b, sizeof(b));
    return buf + sizeof(b);
}

// --- Implementation of Exported Functions ---

BOOL __internal_get_virtual_screens(ScreenInfo* info) {
  return EnumDisplayMonitors(
    NULL,
    NULL,
    &MonitorEnum,
    reinterpret_cast<LPARAM>(info)
  );
}

double get_screen_info(char* inbuf, uint32_t pageNum) {
    PhysicalScreen screenArray[MAX_SCREENS];
    ScreenInfo info;

    // Initialize the struct to pass to the library function
    info.screen = screenArray;
    info.count = 0;
    info.maxCount = MAX_SCREENS;
    info.fromScreen = pageNum * MAX_SCREENS; 
    info.pageNum = pageNum;
    info.more = false;

//    char *buf;
    char* buf = getGMSBuffAddress(inbuf);//Interpret the string address form GMS so it can be managed by C++
    
//    buf = getGMSBuffAddress(inbuf);
    
    // Call the function from the DLL
    if(__internal_get_virtual_screens(&info)) {
        buf = GMSWrite(buf, info.count);
        buf = GMSWrite(buf, info.maxCount);
        buf = GMSWrite(buf, info.fromScreen);
        buf = GMSWrite(buf, info.pageNum);
        buf = GMSWrite(buf, info.autoHideTaskbar);
        buf = GMSWrite(buf, info.more);
        buf = GMSWrite(buf, info.versionMajor);
        buf = GMSWrite(buf, info.versionMinor);
        buf = GMSWrite(buf, info.versionBuild);
        for(int i = 0; i < info.count; i++) {
            buf = GMSWrite(buf, info.screen[i].errorCode);
            buf = GMSWrite(buf, info.screen[i].refreshRate);
            buf = GMSWrite(buf, info.screen[i].isPrimary);

            buf = GMSWrite(buf, info.screen[i].pixelBox.width);
            buf = GMSWrite(buf, info.screen[i].pixelBox.height);

            buf = GMSWrite(buf, info.screen[i].virtualRect.left);
            buf = GMSWrite(buf, info.screen[i].virtualRect.top);
            buf = GMSWrite(buf, info.screen[i].virtualRect.right);
            buf = GMSWrite(buf, info.screen[i].virtualRect.bottom);

            buf = GMSWrite(buf, info.screen[i].workingRect.left);
            buf = GMSWrite(buf, info.screen[i].workingRect.top);
            buf = GMSWrite(buf, info.screen[i].workingRect.right);
            buf = GMSWrite(buf, info.screen[i].workingRect.bottom);

            buf = GMSWrite(buf, info.screen[i].physSize.width);
            buf = GMSWrite(buf, info.screen[i].physSize.height);
            buf = GMSWrite(buf, info.screen[i].physSize.diagonal);

            buf = GMSWrite(buf, info.screen[i].name);
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
            return 0;
        }
    }
    
    // buf is bad, return 1
    return 1;
}

double rezol_ext_get_screen_info(char* inbuf) {
    return get_screen_info(inbuf, 0);
}

double rezol_ext_get_screen_info_page(char* buf, double pageNum) {
    return get_screen_info(buf, pageNum);
}

double rezol_ext_get_window_chrome(char* buf, char* handle) {
    HWND ptr = HWND(handle);
    fprintf(stderr, "Handle = %p\n", ptr);
    return 0;
}


double rezol_ext_get_buffer_size(double which) {
    return rezol_get_buffer_size(which);
}
