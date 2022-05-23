# CMakeList.txt : CMake project for descosmos_gpu_monitor, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("descosmos_gpu_monitor")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# sys include directory
include_directories("C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/shared")
include_directories("C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/um")

# this include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/./utils)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/./config)

set(SRC_SOURCES
	"src/gpuMonitor.h" 
	"src/gpuMonitor.cpp" 
	"config/ntifs.h"   
	"config/phnt.h" 
	"config/phnt_windows.h" 
	"config/ntrtl.h" 
	"config/refp.h" 
	"config/phsup.h")

set(UTILS_SOURCES
	"utils/common.h"
	"utils/getWindowsVersion.h"
	"utils/getWindowsVersion.cpp" 
	"utils/Handles.h" 
	"utils/Handles.cpp"
	"utils/baseup.h"
	"utils/baseup.cpp"
	)


# Add source to this project's executable.
add_executable (descosmos_gpu_monitor "descosmos_gpu_monitor.cpp" ${UTILS_SOURCES} ${SRC_SOURCES})

# TODO: Add tests and install targets if needed.
