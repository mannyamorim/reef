cmake_minimum_required(VERSION 3.5)

project(reef LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

set(TS_FILES
	translation/reef_en_CA.ts
)

set(PROJECT_SOURCES
	src/commit_list.cpp
	src/main.cpp
	src/main_window.cpp
	src/main_window.ui
	src/ref_map.cpp
	src/repository_controller.cpp
	src/text_scroll_area.cpp
	include/commit_list.h
	include/cpp_git.h
	include/error.h
	include/main_window.h
	include/preferences.h
	include/ref_map.h
	include/repository_controller.h
	include/text_scroll_area.h
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

# If using UNIX load the ECM extra-cmake-modules package to find LibGit2
if(UNIX)
	find_package(ECM REQUIRED NO_MODULE)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_FIND_MODULE_DIR})
endif()

# Libgit2 library
if(UNIX)
	find_package(LibGit2)
	if(LIBGIT2_FOUND)
		include_directories(LIBGIT2_INCLUDE_DIR)
		target_link_libraries(reef PRIVATE ${LIBGIT2_LIBRARIES})
	else()
		message(FATAL_ERROR "libgit2 not found")
	endif()
endif()
