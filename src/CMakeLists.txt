# Setup the include dirs
include_directories(
	${Qt${QT_VERSION_MAJOR}Widgets_INCLUDE_DIRS}
	${Qt${QT_VERSION_MAJOR}Test_INCLUDE_DIRS}
	${LIBGIT2_INCLUDE_DIR}
	.
)

# Add the subprojects
add_subdirectory(compat)
add_subdirectory(controller)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(util)

add_subdirectory(test)

# Setup the executable
set(TS_FILES
	../translation/reef_en_CA.ts
)

set(PROJECT_SOURCES
	main.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
	qt_add_executable(reef ${PROJECT_SOURCES} ${TS_FILES})
	qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES})
else()
	add_executable(reef ${PROJECT_SOURCES} ${TS_FILES})
	qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES})
endif()

target_link_libraries(reef PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(reef PRIVATE ${LIBGIT2_LIBRARIES})
target_link_libraries(reef PRIVATE
	compat
	controller
	core
	ui
	util
)

# Install targets
install(TARGETS reef CONFIGURATIONS Release RUNTIME DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/data/ca.amorim.reef.desktop DESTINATION share/applications)
install(FILES ${CMAKE_SOURCE_DIR}/data/ca.amorim.reef.appdata.xml DESTINATION share/metainfo)

# Disable the console on Windows
if(WIN32)
	set_target_properties(reef PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
