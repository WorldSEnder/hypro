add_custom_target(resources)

# Enable ExternalProject CMake module
INCLUDE(ExternalProject)

# Set default ExternalProject root directory
SET(EP_PREFIX_DIR ${CMAKE_BINARY_DIR}/resources )
SET_DIRECTORY_PROPERTIES(PROPERTIES EP_PREFIX ${EP_PREFIX_DIR} )

if(carl_FOUND)
	message("-- Use system version of CArL")
else()
	message("-- Download version of CArL")
	include(carl.cmake)
endif()

if(GTEST_FOUND)
	#set(GTEST_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY} PARENT_SCOPE)
	message("-- Use system version of GTest")
else()
	message("-- Download version of GTest")
	include(gtest.cmake)
endif()


include(glpk.cmake)

