#ifndef SCREEN_UTILS_H
#define SCREEN_UTILS_H

#include <windows.h>
#include <cstdint> // For int32_t

// This macro handles the keywords for exporting from a DLL
// and importing into an executable.
#ifdef SCREEN_UTILS_EXPORTS
    #define SCREEN_API __declspec(dllexport)
#else
    #define SCREEN_API __declspec(dllimport)
#endif

// Custom type definition used in the struct

const int MAX_SCREENS = 8;

// Struct definitions that are part of the public API
struct VirtualRect {
    int32_t left;
    int32_t top;
    int32_t width;
    int32_t height;
};

struct NativeBox {
    int32_t width;
    int32_t height;
};

struct PhysicalSize {
    int32_t width;
    int32_t height;
    int32_t diagonal;
};

struct PhysicalScreen {
    NativeBox       pixelRect;
    int32_t         infoLevel;
    int32_t         refreshRate;
    boolean         isPrimary;
    VirtualRect     virtualRect;
    VirtualRect     taskbarRect;
    VirtualRect     macmenuRect;
    PhysicalSize    physSize;
};

struct ScreenArrayInfo {
    PhysicalScreen* screen;
    int             count;
    int             maxCount;
    boolean         more;
};

// Declare the functions that the library will export.
// The SCREEN_API macro marks them for export.
extern "C" SCREEN_API BOOL ext_get_virtual_screens(ScreenArrayInfo* info);
extern "C" SCREEN_API char* getGMSBuffAddress(char* _GMSBuffPtrStr);
extern "C" SCREEN_API double ext_get_virtual_screens_buffer_size();

#endif // SCREEN_UTILS_H
