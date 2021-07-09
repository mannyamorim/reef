cmake_minimum_required(VERSION 3.5)

project(reef LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

set(TS_FILES
	translation/reef_en_CA.ts
)

set(PROJECT_SOURCES
	src/commit_list.cpp
	src/graph.cpp
	src/main.cpp
	src/main_window.cpp
	src/main_window.ui
	src/ref_map.cpp
	src/repository_controller.cpp
	include/block_allocator.h
	include/commit_list.h
	include/cpp_git.h
	include/error.h
	include/graph.h
	include/main_window.h
	include/preferences.h
	include/reef_string.h
	include/ref_map.h
	include/repository_controller.h
	include/version.h
	${TS_FILES}
)

include_directories(include)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
	qt_add_executable(reef ${PROJECT_SOURCES})
	qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES})
else()
	add_executable(reef ${PROJECT_SOURCES})
	qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES})
endif()

target_link_libraries(reef PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Libgit2 library
find_package(LibGit2)
if(LIBGIT2_FOUND)
	include_directories(LIBGIT2_INCLUDE_DIR)
	target_link_libraries(reef PRIVATE ${LIBGIT2_LIBRARIES})
else()
	message(FATAL_ERROR "libgit2 not found")
endif()

# Disable the console on Windows
if(WIN32)
	set_target_properties(reef PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
