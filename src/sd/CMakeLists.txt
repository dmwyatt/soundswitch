cmake_minimum_required(VERSION 3.20)
project(sd LANGUAGES CXX)

# Link the C runtime statically so sd.exe runs without a Visual C++ redistributable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(sd audiopyle.cpp Device.cpp DevicePrinter.cpp Utils.cpp)

install(TARGETS sd RUNTIME DESTINATION bin)
