cmake_minimum_required(VERSION 3.13)

project(amulet_io LANGUAGES CXX)

set(BUILD_AMULET_IO_TESTS OFF CACHE BOOL "Should tests be built?")

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

# Find C++ files
file(REAL_PATH src SOURCE_PATH)
file(GLOB_RECURSE HEADERS LIST_DIRECTORIES false ${SOURCE_PATH}/amulet/*.hpp)

# Add implementation
add_library(amulet_io INTERFACE)
set_target_properties(amulet_io PROPERTIES FOLDER "CPP")
target_include_directories(amulet_io INTERFACE "src")
target_sources(amulet_io PRIVATE ${HEADERS})
foreach(FILE ${HEADERS})
    file(RELATIVE_PATH REL_PATH ${SOURCE_PATH} ${FILE})
    get_filename_component(GROUP ${REL_PATH} DIRECTORY)
    string(REPLACE "/" "\\" GROUP ${GROUP})
    source_group(${GROUP} FILES ${FILE})
endforeach()

if (BUILD_AMULET_IO_TESTS)
    add_subdirectory(tests)
endif()
