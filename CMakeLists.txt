cmake_minimum_required(VERSION 3.5)

project(reef LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Test REQUIRED)

set(TS_FILES
	translation/reef_en_CA.ts
)

set(PROJECT_SOURCES
	src/compat/cpp_git.h
	src/controller/repository_controller.cpp
	src/controller/repository_controller.h
	src/core/commit_list.cpp
	src/core/commit_list.h
	src/core/graph.cpp
	src/core/graph.h
	src/core/ref_map.cpp
	src/core/ref_map.h
	src/ui/about_window.cpp
	src/ui/about_window.h
	src/ui/about_window.ui
	src/ui/graph_delegate.cpp
	src/ui/graph_delegate.h
	src/ui/main_window.cpp
	src/ui/main_window.h
	src/ui/main_window.ui
	src/util/block_allocator.h
	src/util/error.h
	src/util/preferences.h
	src/util/reef_string.h
	src/util/version.h
	src/main.cpp
	${TS_FILES}
)

include_directories(src)

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
install(TARGETS reef CONFIGURATIONS Release RUNTIME DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/data/ca.amorim.reef.desktop DESTINATION share/applications)
install(FILES ${CMAKE_SOURCE_DIR}/data/ca.amorim.reef.appdata.xml DESTINATION share/metainfo)

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

# Tests with QTest
set(INCLUDE_TESTS OFF CACHE BOOL "Build the test suite.")
if(INCLUDE_TESTS)
	add_executable(reef_test
		src/test/test_graph.cpp
		src/core/graph.cpp
	)
	target_include_directories(reef_test PUBLIC test)
	target_compile_definitions(reef_test PUBLIC __UNIT_TESTING__)
	target_link_libraries(reef_test PRIVATE Qt${QT_VERSION_MAJOR}::Test)
	set_property(TARGET reef_test PROPERTY AUTOMOC ON)

	# Setup target to run the tests
	enable_testing()
	add_test(NAME reef_test_suite COMMAND reef_test)
endif()
