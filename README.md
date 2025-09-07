# GMSVirtualScreen

A GML Extension to query the OS to find the physical sizes of monitors

Windows Only ATM

## How to build as DLL

cmake -B build

cmake --build build --config Release

Test the DLL with `.\build\bin\Release\TestDLL` and check the output

If running a VC command prompt you can use `dumpbin /EXPORTS .\build\bin\Release\GMSVirtualScreen.dll` to examine the result

## ToDo

Add Taskbar detection for Windowed apps
Add Mac version
Add Linux version
