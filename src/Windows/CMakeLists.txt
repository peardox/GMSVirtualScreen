# Set the minimum required version of CMake.
cmake_minimum_required(VERSION 3.15)

# Set the project name and language.
project(ExtVirtualScreen LANGUAGES CXX)

# Set the C++ standard to C++17 for modern features.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This project uses the Windows API, so it will only build on Windows.
if(NOT WIN32)
  message(FATAL_ERROR "This project is Windows-only.")
endif()

# Place all runtime executables (.exe) and shared libraries (.dll)
# into a 'bin' subdirectory inside the build folder.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # For .lib files

# --- 1. Define the Shared Library (DLL) ---

# Create a library target named 'GMSVirtualScreen' from its source files.
# The 'SHARED' keyword specifies that we are building a DLL.
add_library(GMSVirtualScreen SHARED
  screen_utils.cpp
  screen_utils.h
)

# Add the preprocessor definition needed to export symbols from the DLL.
# This definition is PRIVATE, meaning it only applies when compiling
# 'screen_utils' itself. Any target that links to this library will NOT
# inherit this definition, which is exactly what we need for the
# __declspec(dllimport) macro to work correctly in the executable.
target_compile_definitions(GMSVirtualScreen PRIVATE SCREEN_UTILS_EXPORTS)

# Link the library against the Windows User32 library, which is required
# for the EnumDisplayMonitors function.
target_link_libraries(GMSVirtualScreen PUBLIC user32)


# --- 2. Define the Executable ---

# Create an executable target named 'TestInternalDLL' from its source file.
add_executable(TestDLLInternal main.cpp)
add_executable(TestDLLExternal dlltest.cpp)

# Link the executable against our 'GMSVirtualScreen' library.
# CMake handles all the complex details of linking against the import
# library (.lib) on Windows.
target_link_libraries(TestDLLInternal PRIVATE GMSVirtualScreen)
target_link_libraries(TestDLLExternal PRIVATE GMSVirtualScreen)

# This tells CMake where to install the files when we run the install step.
# The install command will create a folder named "install" inside your
# build directory by default.
install(TARGETS TestDLLInternal TestDLLExternal GMSVirtualScreen
  RUNTIME DESTINATION bin  # Install .exe and .dll files to the 'bin' folder
  LIBRARY DESTINATION lib  # Install .lib files (static/import) to 'lib'
  ARCHIVE DESTINATION lib  # Install .lib files (for completeness)
)

# Also install the public header file so other projects could use this library.
install(FILES screen_utils.h DESTINATION include)
