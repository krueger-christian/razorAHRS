cmake_minimum_required (VERSION 2.8)
project(razorAHRS)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_C_FLAGS "-O2 -g")

#Source folders
add_subdirectory(src)

add_executable(example_razorAHRS src/example_razorAHRS.c)
add_executable(example_razorAHRSvirtual src/example_virtualTracker.c)