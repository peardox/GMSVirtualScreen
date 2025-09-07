#include <iostream>
#include <cmath> // For abs()
#include "screen_utils.h" // Include the library's public header

int main() {
    // We can only query for a fixed number of screens.
    PhysicalScreen screenArray[MAX_SCREENS];
    ScreenArrayInfo info;

    // Initialize the struct to pass to the library function
    info.screen = screenArray;
    info.count = 0;
    info.maxCount = MAX_SCREENS;
    info.more = false;

    // Call the function from the DLL
    ext_get_virtual_screens(&info);

    std::wcout << "Buffer Size  : " << ext_get_virtual_screens_buffer_size() << std::endl;
    std::wcout << "Screen Count : " << info.count << std::endl;
    std::wcout << std::endl;

    for (int i = 0; i < info.count; i++) {
        std::wcout << "Screen " << i;
        std::wcout << std::endl;
        std::wcout << "info ";
        std::wcout << ": isPrimary=" << info.screen[i].isPrimary;
        std::wcout << ", refreshRate=" << info.screen[i].refreshRate;
        std::wcout << ", infoLevel=" << info.screen[i].infoLevel;
        std::wcout << std::endl;

        std::wcout << "pixelRect";
        std::wcout << ": Width=" << info.screen[i].pixelRect.width;
        std::wcout << ", Height=" << info.screen[i].pixelRect.height;
        std::wcout << std::endl;

        std::wcout << "virtRect ";
        std::wcout << ": Left=" << info.screen[i].virtualRect.left;
        std::wcout << ", Top=" << info.screen[i].virtualRect.top;
        std::wcout << ", Width=" << info.screen[i].virtualRect.width;
        std::wcout << ", Height=" << info.screen[i].virtualRect.height;
        std::wcout << std::endl;

        std::wcout << "taskRect ";
        std::wcout << ": Left=" << info.screen[i].taskbarRect.left;
        std::wcout << ", Top=" << info.screen[i].taskbarRect.top;
        std::wcout << ", Width=" << info.screen[i].taskbarRect.width;
        std::wcout << ", Height=" << info.screen[i].taskbarRect.height;
        std::wcout << std::endl;

        std::wcout << "menuRect ";
        std::wcout << ": Left=" << info.screen[i].macmenuRect.left;
        std::wcout << ", Top=" << info.screen[i].macmenuRect.top;
        std::wcout << ", Width=" << info.screen[i].macmenuRect.width;
        std::wcout << ", Height=" << info.screen[i].macmenuRect.height;
        std::wcout << std::endl;

        std::wcout << "physSize ";
        std::wcout << ": Width=" << info.screen[i].physSize.width;
        std::wcout << ", Height=" << info.screen[i].physSize.height;
        std::wcout << ", Diagonal=" << info.screen[i].physSize.diagonal;
        std::wcout << std::endl;

        std::wcout << std::endl;
    }

    return 0;
}