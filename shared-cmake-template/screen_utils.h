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
typedef int32_t wordbool;

// Struct definitions that are part of the public API
struct VirtScreen {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
  HMONITOR monitorHandle;
};

struct ScreenArrayInfo {
  VirtScreen* Screen;
  int Primary;
  wordbool More;
  int Count;
  int Padding;
  int MaxCount;
};

// Declare the functions that the library will export.
// The SCREEN_API macro marks them for export.
extern "C" SCREEN_API BOOL ext_get_virtual_screens(ScreenArrayInfo* info);
extern "C" SCREEN_API char* getGMSBuffAddress(char* _GMSBuffPtrStr);

#endif // SCREEN_UTILS_H
