#include <iostream>
#include <cmath> // For abs()
#include "screen_utils.h" // Include the library's public header

int main() {
  // We can only query for a fixed number of screens.
  const int MAX_SCREENS = 4;
  VirtScreen screenArray[MAX_SCREENS];
  ScreenArrayInfo info;

  // Initialize the struct to pass to the library function
  info.Screen = screenArray;
  info.Count = 0;
  info.MaxCount = MAX_SCREENS;
  info.More = false;

  // Call the function from the DLL
  ext_get_virtual_screens(&info);

  std::wcout << "Screen Count: " << info.Count << std::endl;
  if (info.More) {
    std::wcout << "Warning: More screens exist than buffer could hold."
               << std::endl;
  }

  for (int i = 0; i < info.Count; i++) {
    std::wcout << "Screen " << i;
    std::wcout << ": Left=" << info.Screen[i].left;
    std::wcout << ", Top=" << info.Screen[i].top;
    std::wcout << ", Width=" << abs(info.Screen[i].right - info.Screen[i].left);
    std::wcout << ", Height=" << abs(info.Screen[i].bottom - info.Screen[i].top);
    std::wcout << ", Handle=0x" << std::hex
               << (uintptr_t)info.Screen[i].monitorHandle << std::dec;
    std::wcout << std::endl;
  }

  return 0;
}