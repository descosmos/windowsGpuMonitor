# CMakeList.txt : CMake project for descosmos_gpu_monitor, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("descosmos_gpu_monitor")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/./utils)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/./config)

set(SRC_SOURCES
	"src/gpuMonitor.h" 
	"src/gpuMonitor.cpp")

set(UTILS_SOURCES
	"./utils/common.h"
	"./utils/getWindowsVersion.h"
	"./utils/getWindowsVersion.cpp")


# Add source to this project's executable.
add_executable (descosmos_gpu_monitor "descosmos_gpu_monitor.cpp" "descosmos_gpu_monitor.h" ${UTILS_SOURCES} ${SRC_SOURCES})

# TODO: Add tests and install targets if needed.
