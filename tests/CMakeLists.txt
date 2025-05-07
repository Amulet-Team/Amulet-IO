cmake_minimum_required(VERSION 3.13)

project(amulet_io_tests LANGUAGES CXX)

# Set C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set platform variables
if (WIN32)
	# set windows 7 as the minimum version
	add_definitions(-D_WIN32_WINNT=0x0601)
elseif(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
else()
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Find dependencies
find_package(pybind11 CONFIG REQUIRED)
find_package(amulet_io CONFIG REQUIRED)
find_package(amulet_test_utils CONFIG REQUIRED)

pybind11_add_module(_test_amulet_io "test_amulet_io/_test_amulet_io.py.cpp")
target_compile_definitions(_test_amulet_io PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES)
target_link_libraries(_test_amulet_io PRIVATE amulet_io)
target_link_libraries(_test_amulet_io PRIVATE amulet_test_utils)

# Install
install(TARGETS _test_amulet_io DESTINATION ${TEST_AMULET_IO_DIR})
