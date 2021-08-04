cmake_minimum_required(VERSION 3.5)

project(reef LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
	src/graph_delegate.cpp
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
	include/graph_delegate.h
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
set_property(TARGET reef PROPERTY AUTOUIC ON)
set_property(TARGET reef PROPERTY AUTOMOC ON)
set_property(TARGET reef PROPERTY AUTORCC ON)

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

# Tests with googletest
set(INCLUDE_TESTS OFF CACHE BOOL "Build the test suite.")
if(INCLUDE_TESTS)
	# Prevent overriding the parent project's compiler/linker settings on Windows
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

	# Add googletest directly to our build. This defines the gtest and gtest_main targets.
	set(GTEST_SOURCE_DIR /usr/src/googletest/ CACHE PATH "Path to the googletest sources. Defaults to /usr/src/googletest. May have to change if your distro places the sources in a different folder.")
	add_subdirectory(
		${GTEST_SOURCE_DIR}
		googletest-bin
		EXCLUDE_FROM_ALL
	)
        include_directories(${GTEST_SOURCE_DIR}/googletest/include)

	# Now simply link against gtest or gtest_main as needed
	add_executable(reef_test
		test/main.cpp
		test/test_graph.cpp
		src/graph.cpp
	)
        target_include_directories(reef_test PUBLIC test)
	target_compile_definitions(reef_test PUBLIC __UNIT_TESTING__)
	target_link_libraries(reef_test gtest_main)

	# Setup target to run the tests
	enable_testing()
	add_test(NAME reef_test_suite COMMAND reef_test)
endif()
