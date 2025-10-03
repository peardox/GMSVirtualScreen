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
constexpr size_t MONITOR_NAME_BUFFER_SIZE = 64;

enum REZOL_DATA_BUFFER {
    SCREENINFOHEADER,
    SCREENINFO,
    PHYSICALSCREEN,
    WINDOWCHROME
};

// Struct definitions that are part of the public API
struct GMSRect {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};

struct GMSBox {
    int32_t width;
    int32_t height;
};

struct PhysicalSize {
    int32_t width;
    int32_t height;
    int32_t diagonal;
};

struct PhysicalScreen {
    int32_t         errorCode;
    int32_t         refreshRate;
    int32_t         isPrimary;
    GMSBox          pixelBox;
    GMSRect         virtualRect;
    GMSRect         workingRect;
    PhysicalSize    physSize;
	char   			name[MONITOR_NAME_BUFFER_SIZE];
};

struct ScreenInfo {
    int32_t count;
    int32_t maxCount;
    int32_t fromScreen;
    int32_t pageNum;
    int32_t autoHideTaskbar; // passed to gml as 4 int32_t for 4 byte alignment
    boolean more; // 8 bit
    uint8_t versionMajor = GMSVersionMajor; // 8 bit
    uint8_t versionMinor = GMSVersionMinor; // 8 bit
    uint8_t versionBuild = GMSVersionBuild; // 8 bit
    PhysicalScreen* screen;
    uint32_t fourcc      = GMEX;
};

struct WindowChrome {
    GMSRect  outerRect;
    GMSRect  innerRect;
    uint32_t fourcc = GMEX;
};

// Declare the functions that the library will export.
// The SCREEN_API macro marks them for export.
extern "C" SCREEN_API double rezol_ext_get_buffer_size(double which);
extern "C" SCREEN_API double rezol_ext_get_screen_info(char* buf);
extern "C" SCREEN_API double rezol_ext_get_screen_info_page(char* buf, double pageNum);
extern "C" SCREEN_API double rezol_ext_get_window_chrome(char* buf, char* handle);
extern "C" SCREEN_API BOOL __internal_get_virtual_screens(ScreenInfo* info);

#endif // SCREEN_UTILS_H
