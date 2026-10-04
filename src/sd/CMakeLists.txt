cmake_minimum_required(VERSION 3.25)
project(sd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Link the C runtime statically so sd.exe runs without a Visual C++ redistributable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(sd_core STATIC Device.cpp Endpoints.cpp Options.cpp)
target_include_directories(sd_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(sd_core PUBLIC /W4 /permissive- /utf-8)
target_compile_definitions(sd_core PUBLIC UNICODE _UNICODE NOMINMAX)

add_executable(sd audiopyle.cpp)
target_link_libraries(sd PRIVATE sd_core)

install(TARGETS sd RUNTIME DESTINATION bin)

include(CTest)
if(BUILD_TESTING)
	include(FetchContent)
	set(DOCTEST_NO_INSTALL ON)
	FetchContent_Declare(doctest
		GIT_REPOSITORY https://github.com/doctest/doctest.git
		GIT_TAG v2.5.3
		GIT_SHALLOW TRUE
		SYSTEM)
	FetchContent_MakeAvailable(doctest)

	add_executable(sd_tests tests/DeviceTests.cpp tests/EndpointsTests.cpp tests/OptionsTests.cpp)
	target_link_libraries(sd_tests PRIVATE sd_core doctest::doctest_with_main)
	add_test(NAME sd_tests COMMAND sd_tests)

	add_test(NAME sd_rejects_unknown_argument COMMAND sd -x)
	set_tests_properties(sd_rejects_unknown_argument PROPERTIES WILL_FAIL TRUE)
endif()
