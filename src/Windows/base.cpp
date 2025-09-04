#include <windows.h>
#include <cstring>
#include <string>
#include <iostream>

using namespace std;

typedef int32_t wordbool;

struct VirtScreen {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
    HMONITOR monitorHandle;
};

struct ScreenArrayInfo
{
    VirtScreen* Screen;
    int Primary;
    wordbool More;
    int Count;
    int Padding;
    int MaxCount;
}; 

static VirtScreen RectToScreen(HMONITOR hMon, LPRECT lprcMonitor) {
    VirtScreen screen;
    screen.left = lprcMonitor->left;
    screen.top = lprcMonitor->top;
    screen.right = lprcMonitor->right;
    screen.bottom = lprcMonitor->bottom;
    screen.monitorHandle = hMon;
    return screen;
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

static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
{
    ScreenArrayInfo* info = reinterpret_cast<ScreenArrayInfo*>(pData);
    if (info->Count == info->MaxCount) {
        info->More = true;
        return false;
    }
    info->Screen[info->Count] = RectToScreen(hMon, lprcMonitor);
    info->Count++;
    return true;
}

BOOL ext_get_virtual_screens(ScreenArrayInfo *info) {
  return EnumDisplayMonitors(NULL, NULL, &MonitorEnum, reinterpret_cast<LPARAM>(info));
}

int main() {
    VirtScreen screenArray[4];
    ScreenArrayInfo info;
    info.Screen = screenArray;
    info.Count = 0;
    info.MaxCount = 4;

    if (ext_get_virtual_screens(&info)) {
        std::wcout << "Screen Count: " << info.Count << std::endl;
        for (int i = 0; i < info.Count; i++) {
            std::wcout << "Screen : " << i;
            std::wcout << " : Left : " << info.Screen[i].left;
            std::wcout << ", Top : " << info.Screen[i].top;
            std::wcout << ", Width : " << abs(info.Screen[i].right - info.Screen[i].left);
            std::wcout << ", Bottom : " << abs(info.Screen[i].bottom - info.Screen[i].top);
            std::wcout << ", Handle : " << (int64_t)info.Screen[i].monitorHandle;
            std::wcout << std::endl;

        }
    }
}
