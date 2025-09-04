# CMakeList.txt : CMake project for GMSVirtualScreen, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 4.0)
# Add source to this project's executable.
if(GMS_SHARED)
    add_compile_options(-DGMS_SHARED)
	add_library (GMSVirtualScreen SHARED "GMSVirtualScreen.cpp" )
else()
	add_executable (GMSVirtualScreen "GMSVirtualScreen.cpp" )
	add_executable (refresh "refresh.cpp" )
	add_executable (base "base.cpp" )
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET GMSVirtualScreen PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
