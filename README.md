# GMSVirtualScreen

A GML Extension to query the OS to find the physical sizes of monitors

Windows Only ATM

## How to build as DLL

cmake -B build -DGMS_SHARED=ON

cmake --build build --config Release

If running a VC command prompt you can use `dumpbin /EXPORTS build\Release\GMSVirtualScreen.dll` to examine the result

## How to build as Test EXE

cmake -B build

cmake --build build --config Release

Then run `build\Release\GMSVirtualScreen` to test
