#include "screen_utils.h"
#include <string> // For stoull

// This is an internal helper function, so it's marked 'static'
// and is not declared in the header file.
static VirtScreen RectToScreen(HMONITOR hMon, LPRECT lprcMonitor) {
  VirtScreen screen;
  screen.left = lprcMonitor->left;
  screen.top = lprcMonitor->top;
  screen.right = lprcMonitor->right;
  screen.bottom = lprcMonitor->bottom;
  screen.monitorHandle = hMon;
  return screen;
}

// This is the internal callback function for EnumDisplayMonitors.
// It is also marked 'static' and kept private to this file.
static BOOL CALLBACK MonitorEnum(
  HMONITOR hMon,
  HDC hdc,
  LPRECT lprcMonitor,
  LPARAM pData
) {
  ScreenArrayInfo* info = reinterpret_cast<ScreenArrayInfo*>(pData);
  if (info->Count == info->MaxCount) {
    info->More = true;
    return false;
  }
  info->Screen[info->Count] = RectToScreen(hMon, lprcMonitor);
  info->Count++;
  return true;
}

// --- Implementation of Exported Functions ---

BOOL ext_get_virtual_screens(ScreenArrayInfo* info) {
  return EnumDisplayMonitors(
    NULL,
    NULL,
    &MonitorEnum,
    reinterpret_cast<LPARAM>(info)
  );
}

char* getGMSBuffAddress(char* _GMSBuffPtrStr) {
  size_t GMSBuffLongPointer = std::stoull(_GMSBuffPtrStr, NULL, 16);
  return (char*)GMSBuffLongPointer;
}