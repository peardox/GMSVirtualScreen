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
const byte GMSVersionMajor = 0;
const byte GMSVersionMinor = 1;
const byte GMSVersionBuild = 1;

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
    int             count;
    int             maxCount;
    boolean         more;
    unsigned char   versionMajor = GMSVersionMajor;
    unsigned char   versionMinor = GMSVersionMinor;
    unsigned char   versionBuild = GMSVersionBuild;
    PhysicalScreen* screen;
};

// Declare the functions that the library will export.
// The SCREEN_API macro marks them for export.
extern "C" SCREEN_API double ext_get_virtual_screens(char* buf);
extern "C" SCREEN_API double ext_get_virtual_screens_buffer_size();
extern "C" SCREEN_API BOOL __internal_get_virtual_screens(ScreenArrayInfo* info);

#endif // SCREEN_UTILS_H
