# Reef - TUI Client for Git
# Copyright (C) 2020 Emmanuel Mathi-Amorim
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.11)
project(reef VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include (ExternalProject)
# Include the module for finding curses
set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
include(FindCurses)
# Set the modules path so we can load the find module for libgit2
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

include_directories(include)
file(GLOB SOURCES "src/*.cpp" "include/*.h")

if(WIN32)
	list(APPEND SOURCES "resources/reef.rc")
endif()

add_executable(reef ${SOURCES})
target_compile_features(reef PRIVATE cxx_std_14)
install(TARGETS reef CONFIGURATIONS Release RUNTIME DESTINATION bin)

# Tests with googletest

# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines the gtest and gtest_main targets.
if(UNIX)
	set(GTEST_SOURCE_DIR /usr/src/googletest/ CACHE PATH "Path to the googletest sources. Defaults to /usr/src/googletest. May have to change if your distro places the sources in a different folder.")
else()
	set(GTEST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd_party/googletest/)
endif()
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

# Force Unicode support for the source files
if(MSVC)
	target_compile_options(reef_test PUBLIC /utf-8)
endif()

# Curses library
if(UNIX)
	find_package(Curses)
	if(CURSES_FOUND)
		include_directories(CURSES_INCLUDE_DIRS)
		target_compile_definitions(reef PUBLIC _XOPEN_SOURCE_EXTENDED)
		target_link_libraries(reef ${CURSES_LIBRARIES})
	else()
		message(FATAL_ERROR "curses not found")
	endif()
else()
	set(PDC_SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rd_party/pdcurses/")
	set(PDC_BINARY_DIR "pdcurses-bin")
	set(PDC_INCLUDE_DIR "${PDC_SOURCE_DIR}")

	ExternalProject_Add(pdcurses
		SOURCE_DIR ${PDC_SOURCE_DIR}
		BINARY_DIR ${PDC_BINARY_DIR}
		CMAKE_ARGS -DPDCDEBUG=OFF -DPDC_BUILD_SHARED=ON -DPDC_WIDE=ON -DPDC_SDL2_BUILD=OFF -DPDC_SDL2_DEPS_BUILD=OFF
		STEP_TARGETS build
		EXCLUDE_FROM_ALL TRUE
	)

	add_compile_definitions(PDC_DLL_BUILD PDC_WIDE)
	include_directories(${PDC_INCLUDE_DIR})
	add_dependencies(reef pdcurses-build)

	set(PDC_LIB_OUTPUT "${PDC_BINARY_DIR}/wincon/$<CONFIG>/pdcurses")
	target_link_libraries(reef ${PDC_LIB_OUTPUT})
	add_custom_command(TARGET reef POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${PDC_LIB_OUTPUT}.dll ${CMAKE_BINARY_DIR}/$<CONFIG>
	)
endif()

# Libgit2 library
if(UNIX)
	find_package(Libgit2)
	if(LIBGIT2_FOUND)
		include_directories(LIBGIT2_INCLUDE_DIR)
		target_link_libraries(reef ${LIBGIT2_LIBRARIES})
	else()
		message(FATAL_ERROR "libgit2 not found")
	endif()
else()
	set(LIBGIT2_SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rd_party/libgit2/")
	set(LIBGIT2_BINARY_DIR "libgit2-bin")
	set(LIBGIT2_INCLUDE_DIR ${LIBGIT2_SOURCE_DIR}/include)

	ExternalProject_Add(libgit2
		SOURCE_DIR ${LIBGIT2_SOURCE_DIR}
		BINARY_DIR ${LIBGIT2_BINARY_DIR}
		STEP_TARGETS build
		EXCLUDE_FROM_ALL TRUE
	)

	include_directories(${LIBGIT2_INCLUDE_DIR})
	add_dependencies(reef libgit2-build)

	set(LIBGIT2_LIB_OUTPUT "${LIBGIT2_BINARY_DIR}/$<CONFIG>/git2")
	target_link_libraries(reef ${LIBGIT2_LIB_OUTPUT})
	add_custom_command(TARGET reef POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${LIBGIT2_LIB_OUTPUT}.dll ${CMAKE_BINARY_DIR}/$<CONFIG>
	)
endif()

# utf8proc library
if(UNIX)
	find_package(UTF8PROC)
	if(UTF8PROC_FOUND)
		include_directories(UTF8PROC_INCLUDE_DIRS)
		target_link_libraries(reef ${UTF8PROC_LIBRARIES})
		target_link_libraries(reef_test ${UTF8PROC_LIBRARIES})
	else()
		message(FATAL_ERROR "utf8proc not found")
	endif()
else()
	set(UTF8PROC_SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rd_party/utf8proc/")
	set(UTF8PROC_BINARY_DIR "utf8proc-bin")

	ExternalProject_Add(utf8proc
		SOURCE_DIR ${UTF8PROC_SOURCE_DIR}
		BINARY_DIR ${UTF8PROC_BINARY_DIR}
		CMAKE_ARGS -DBUILD_SHARED_LIBS=ON
		STEP_TARGETS build
		EXCLUDE_FROM_ALL TRUE
	)

	include_directories(${UTF8PROC_SOURCE_DIR})
	add_dependencies(reef utf8proc-build)

	set(UTF8PROC_LIB_OUTPUT "${UTF8PROC_BINARY_DIR}/$<CONFIG>/utf8proc")
	target_link_libraries(reef ${UTF8PROC_LIB_OUTPUT})
	target_link_libraries(reef_test ${UTF8PROC_LIB_OUTPUT})
	add_custom_command(TARGET reef POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${UTF8PROC_LIB_OUTPUT}.dll ${CMAKE_BINARY_DIR}/$<CONFIG>
	)
	add_custom_command(TARGET reef_test POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${UTF8PROC_LIB_OUTPUT}.dll ${CMAKE_BINARY_DIR}/$<CONFIG>
	)
endif()
