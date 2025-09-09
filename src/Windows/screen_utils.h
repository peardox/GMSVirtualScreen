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

constexpr int     MAX_SCREENS = 8;
constexpr uint8_t GMSVersionMajor = 0;
constexpr uint8_t GMSVersionMinor = 1;
constexpr uint8_t GMSVersionBuild = 1;
constexpr uint32_t GMEX = 0x474D4558; // "GMEX"

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
    int32_t         infoLevel;
    int32_t         refreshRate;
    int32_t         isPrimary;
    NativeBox       pixelRect;
    VirtualRect     virtualRect;
    VirtualRect     taskbarRect;
    VirtualRect     macmenuRect;
    PhysicalSize    physSize;
};

struct ScreenArrayInfo {
    int     count;
    int     maxCount;
    int32_t autoHideTaskbar; // passed to gml as 4 int32_t for 4 byte alignment
    boolean more; // 8 bit
    uint8_t versionMajor = GMSVersionMajor; // 8 bit
    uint8_t versionMinor = GMSVersionMinor; // 8 bit
    uint8_t versionBuild = GMSVersionBuild; // 8 bit
    PhysicalScreen* screen;
    uint32_t fourcc      = GMEX;
};

// Declare the functions that the library will export.
// The SCREEN_API macro marks them for export.
extern "C" SCREEN_API double ext_get_virtual_screens(char* buf);
extern "C" SCREEN_API double ext_get_virtual_screens_buffer_size();
extern "C" SCREEN_API double ext_get_screens_data_size();
extern "C" SCREEN_API BOOL __internal_get_virtual_screens(ScreenArrayInfo* info);

#endif // SCREEN_UTILS_H
