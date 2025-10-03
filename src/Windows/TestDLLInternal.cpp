#include <iostream>
#include <cmath> // For abs()
#include "screen_utils.h" // Include the library's public header

int main() {
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	
    // We can only query for a fixed number of screens.
    PhysicalScreen screenArray[MAX_SCREENS];
    ScreenInfo info;

    // Initialize the struct to pass to the library function
    info.screen = screenArray;
    info.count = 0;
    info.maxCount = MAX_SCREENS;
    info.more = false;

    // Call the function from the DLL
    __internal_get_virtual_screens(&info);

    std::wcout << "Buffer Size  : " << rezol_ext_get_buffer_size(SCREENINFO) << std::endl;
    
    std::wcout << "PhysicalScreen  : " << rezol_ext_get_buffer_size(PHYSICALSCREEN) << std::endl;
    
    std::wcout << "Screen Count : " << info.count << std::endl;
    std::wcout << std::endl;

    for (int i = 0; i < info.count; i++) {
        std::wcout << "Screen " << i;
        std::wcout << std::endl;
        std::wcout << "info ";
        std::wcout << ": isPrimary=" << info.screen[i].isPrimary;
        std::wcout << ", refreshRate=" << info.screen[i].refreshRate;
        std::wcout << ", errorCode=" << info.screen[i].errorCode;
        std::wcout << std::endl;

        std::wcout << "pixelBox ";
        std::wcout << ": Width=" << info.screen[i].pixelBox.width;
        std::wcout << ", Height=" << info.screen[i].pixelBox.height;
        std::wcout << std::endl;

        std::wcout << "virtRect ";
        std::wcout << ": Left=" << info.screen[i].virtualRect.left;
        std::wcout << ", Top=" << info.screen[i].virtualRect.top;
        std::wcout << ", Right=" << info.screen[i].virtualRect.right;
        std::wcout << ", Bottom=" << info.screen[i].virtualRect.bottom;
        std::wcout << std::endl;

        std::wcout << "workRect ";
        std::wcout << ": Left=" << info.screen[i].workingRect.left;
        std::wcout << ", Top=" << info.screen[i].workingRect.top;
        std::wcout << ", Right=" << info.screen[i].workingRect.right;
        std::wcout << ", Bottom=" << info.screen[i].workingRect.bottom;
        std::wcout << std::endl;

        std::wcout << "physSize ";
        std::wcout << ": Width=" << info.screen[i].physSize.width;
        std::wcout << ", Height=" << info.screen[i].physSize.height;
        std::wcout << ", Diagonal=" << info.screen[i].physSize.diagonal;
        std::wcout << std::endl;

        std::wcout << "DispName : " << info.screen[i].name;
        std::wcout << std::endl;

        std::wcout << std::endl;
    }

    return 0;
}