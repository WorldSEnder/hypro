if(${STATICLIB_SWITCH} STREQUAL "OFF")
	include_directories(${GTEST_INCLUDE_DIR})

	################################
	# Unit Tests
	################################

	add_executable(runDatastructureTests
		HalfspaceTest.cpp
		PointTest.cpp
		VertexTest.cpp
		VertexContainerTest.cpp
		HybridAutomataTest.cpp
	)

	add_dependencies(runDatastructureTests googletest)

	target_link_libraries(runDatastructureTests
							${PROJECT_NAME}
							${GTEST_LIBRARIES}
							)

	cotire(runDatastructureTests)

	add_test( NAME datastructures COMMAND runDatastructureTests )
endif()
