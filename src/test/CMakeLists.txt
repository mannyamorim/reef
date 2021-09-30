# Tests with QTest
if(INCLUDE_TESTS)
	# Setup test executable
	add_executable(reef_test
		test_graph.cpp
	)

	target_link_libraries(reef_test PRIVATE Qt${QT_VERSION_MAJOR}::Test)
	target_link_libraries(reef_test PRIVATE ${LIBGIT2_LIBRARIES})
	target_link_libraries(reef_test PRIVATE core)

	set_property(TARGET reef_test PROPERTY AUTOMOC ON)

	# Setup target to run the tests
	add_test(NAME reef_test_suite COMMAND reef_test)
endif()
