# GMSVirtualScreen

A GML Extension to query the OS to find the physical sizes of monitors

Windows Only ATM

## How to build

cmake -B build

cmake --build build --config Release

Test the DLL with `.\build\bin\Release\TestDLLInternal` and check the output

If running a VC command prompt you can use `dumpbin /EXPORTS .\build\bin\Release\GMSVirtualScreen.dll` to examine the result

## Exported Library Functions

### real ext_get_virtual_screens_buffer_size();

Returns size of buffer required to hold results

### real ext_get_virtual_screens(gm_buf);

Fills gm_buf with a ScreenArrayInfo (see screen_utils.h). Last 4 buffer bytes will be a fourCC of "GMEX" for error checking.

### real ext_get_screens_data_size();

Returns size of a PhysicalScreen (as will may be many) - useful for skipping over empties

## ToDo

- Add Taskbar detection for Windowed apps
- Add Mac version
- Add Linux version
