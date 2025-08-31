// GMSVirtualScreen.cpp : Defines the entry point for the application.
//

#ifdef _WIN64
#define GMS2EXPORT extern "C" __declspec(dllexport)
#else
#define GMS2EXPORT extern "C"
#endif
#include <Windows.h>
#include <cstring>
#include <string>

#ifndef GMS_SHARED
#include <iostream>
#endif

using namespace std;

struct VirtScreen {
    LONG left;
    LONG top;
    LONG width;
    LONG height;
    HMONITOR monitorHandle;
};

struct ScreenArrayInfo
{
    VirtScreen* Screen;
    int Primary;
    bool More;
    int Count;
    int MaxCount;
}; 

static VirtScreen RectToScreen(HMONITOR hMon, LPRECT lprcMonitor) {
    VirtScreen screen;
    screen.left = lprcMonitor->left;
    screen.top = lprcMonitor->top;
    screen.width = abs(lprcMonitor->right - lprcMonitor->left);
    screen.height = abs(lprcMonitor->bottom - lprcMonitor->top);
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

#ifdef GMS_SHARED
GMS2EXPORT double ext_get_virtual_screens(char* _GMSBuffPtrStr) {
    const int MaxScreenCount = 4;
    char* _GMSBuffer = getGMSBuffAddress(_GMSBuffPtrStr);//Interpret the string address form GMS so it can be managed by C++
    //you can now do memcpy operations (or any other mem operations) to the buffer :)

    unsigned int currentWriteOffset = 0;//A value to know the offset to where we need to write in the buffer

    VirtScreen screenArray[MaxScreenCount];
    ScreenArrayInfo info;
    info.Screen = screenArray;
    info.Primary = -1;
    info.More = false;
    info.Count = 0;
    info.MaxCount = MaxScreenCount;

    if (EnumDisplayMonitors(NULL, NULL, &MonitorEnum, reinterpret_cast<LPARAM>(&info)) != 0) {
        memcpy(&_GMSBuffer[currentWriteOffset], &info.Count, sizeof(info.Count));//Write the unsigned short int
        currentWriteOffset += sizeof(info.Count);//Increase the offset to write to the right position of the buffer
        memcpy(&_GMSBuffer[currentWriteOffset], &info.Primary, sizeof(info.Primary));//Write the unsigned short int
        currentWriteOffset += sizeof(info.Primary);//Increase the offset to write to the right position of the buffer
        memcpy(&_GMSBuffer[currentWriteOffset], &info.More, sizeof(info.More));//Write the unsigned short int
        currentWriteOffset += sizeof(info.More);//Increase the offset to write to the right position of the buffer
        for (int i = 0; i < info.Count; i++) {
            memcpy(&_GMSBuffer[currentWriteOffset], &info.Screen[i].left, sizeof(info.Screen[i].left));//Write the unsigned int
            currentWriteOffset += sizeof(info.Screen[i].left);//Increase the offset to write to the right position of the buffer

            memcpy(&_GMSBuffer[currentWriteOffset], &info.Screen[i].top, sizeof(info.Screen[i].top));//Write the unsigned int
            currentWriteOffset += sizeof(info.Screen[i].top);//Increase the offset to write to the right position of the buffer

            memcpy(&_GMSBuffer[currentWriteOffset], &info.Screen[i].width, sizeof(info.Screen[i].width));//Write the unsigned int
            currentWriteOffset += sizeof(info.Screen[i].width);//Increase the offset to write to the right position of the buffer

            memcpy(&_GMSBuffer[currentWriteOffset], &info.Screen[i].height, sizeof(info.Screen[i].height));//Write the unsigned int
            currentWriteOffset += sizeof(info.Screen[i].height);//Increase the offset to write to the right position of the buffer

            memcpy(&_GMSBuffer[currentWriteOffset], &info.Screen[i].monitorHandle, sizeof(info.Screen[i].monitorHandle));//Write the unsigned int
            currentWriteOffset += sizeof(info.Screen[i].monitorHandle);//Increase the offset to write to the right position of the buffer
        }
        /* Could optionally blank remaining entries
        for (int i = info.Count; i < info.MaxCount; i++) {
            memset(&_GMSBuffer[currentWriteOffset], (LONG)0, sizeof(LONG));//Write null entries
            currentWriteOffset += sizeof(LONG);
            memset(&_GMSBuffer[currentWriteOffset], (LONG)0, sizeof(LONG));//Write null entries
            currentWriteOffset += sizeof(LONG);
            memset(&_GMSBuffer[currentWriteOffset], (LONG)0, sizeof(LONG));//Write null entries
            currentWriteOffset += sizeof(LONG);
            memset(&_GMSBuffer[currentWriteOffset], (LONG)0, sizeof(LONG));//Write null entries
            currentWriteOffset += sizeof(LONG);
            memset(&_GMSBuffer[currentWriteOffset], (LONG64) 0, sizeof(LONG64));//Write null entries
            currentWriteOffset += sizeof(HMONITOR);
        }
        */

        return 1;
    } else {
        return 0;
    }

}
#else
int main() {
    VirtScreen screenArray[4];
    ScreenArrayInfo info;
    info.Screen = screenArray;
    info.Count = 0;
    info.MaxCount = 4;

    if (EnumDisplayMonitors(NULL, NULL, &MonitorEnum, reinterpret_cast<LPARAM>(&info)) != 0) {
        std::wcout << "Screen Count: " << info.Count << std::endl;
        for (int i = 0; i < info.Count; i++) {
            std::wcout << "Screen : " << i;
            std::wcout << " : Left : " << info.Screen[i].left;
            std::wcout << ", Top : " << info.Screen[i].top;
            std::wcout << ", Width : " << info.Screen[i].width;
            std::wcout << ", Height : " << info.Screen[i].height;
            std::wcout << ", Handle : " << (int64_t)info.Screen[i].monitorHandle;
            std::wcout << std::endl;

        }
    }
}
#endif
