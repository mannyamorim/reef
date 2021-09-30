cmake_minimum_required(VERSION 3.5)

project(reef LANGUAGES CXX)

# Set properties
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(INCLUDE_TESTS OFF CACHE BOOL "Build the test suite.")

# Load QT Libraries
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Test REQUIRED)

# Load libgit2 library
find_package(LibGit2)
if(NOT LIBGIT2_FOUND)
	message(FATAL_ERROR "libgit2 not found")
endif()

# Enable testing
if(INCLUDE_TESTS)
	enable_testing()
endif()

# Include the project
add_subdirectory(src)
