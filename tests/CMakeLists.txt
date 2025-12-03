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
elseif(UNIX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
else()
    message( FATAL_ERROR "Unsupported platform. Please submit a pull request to support this platform." )
endif()

if (MSVC)
    add_definitions("/MP")
endif()

# Find dependencies
if (NOT TARGET pybind11::module)
    find_package(pybind11 CONFIG REQUIRED)
endif()
if (NOT TARGET amulet_test_utils)
    find_package(amulet_test_utils CONFIG REQUIRED)
endif()
if (NOT TARGET amulet_io)
    find_package(amulet_io CONFIG REQUIRED)
endif()

pybind11_add_module(_test_amulet_io "test_amulet_io/_test_amulet_io.py.cpp")
set_target_properties(_test_amulet_io PROPERTIES FOLDER "Tests")
target_compile_definitions(_test_amulet_io PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES)
target_link_libraries(_test_amulet_io PRIVATE amulet_io)
target_link_libraries(_test_amulet_io PRIVATE amulet_test_utils)

# Install
install(TARGETS _test_amulet_io DESTINATION "${CMAKE_CURRENT_LIST_DIR}/test_amulet_io")
