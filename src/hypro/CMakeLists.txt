
# Create config.h by adding cmake defines dependend on option switches
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/flags.h.in ${CMAKE_CURRENT_SOURCE_DIR}/flags.h )

########################################################################################

file(GLOB_RECURSE headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
file(GLOB_RECURSE sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
#MESSAGE(STATUS "Globbed header sources: ${sources}")

if(STATICLIB_SWITCH)
	set(STATICLIB "STATIC")
else()
	set(STATICLIB "SHARED")
endif()

########################################################################################

if(STATICLIB_SWITCH)
	message(STATUS "linking static")
	add_library(${PROJECT_NAME}_static STATIC ${sources})
	set_target_properties( hypro_static PROPERTIES
		# create *nix style library versions + symbolic links
		VERSION "${PROJECT_VERSION}"
		SOVERSION "${PROJECT_VERSION}"
		# allow creating static and shared libs without conflicts
		CLEAN_DIRECT_OUTPUT 1
		# avoid conflicts between library and binary target names
		OUTPUT_NAME hypro
		LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
		ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/src"
		LINKER_LANGUAGE CXX
	)
	#message("hypro libraries static: " ${hypro_LIBRARIES_STATIC})
	#message("hypro include dirs: " ${hypro_INCLUDE_DIRS})
	#message("hypro sources: " ${sources})
	#message("hypro headers: " ${headers})
	target_link_libraries(${PROJECT_NAME}_static PUBLIC ${hypro_LIBRARIES_STATIC})
	target_include_directories(${PROJECT_NAME}_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${hypro_INCLUDE_DIRS})
else()
	message(STATUS "linking dynamic")
	add_library(${PROJECT_NAME} SHARED ${sources})
	set_target_properties( ${PROJECT_NAME} PROPERTIES
		# create *nix style library versions + symbolic links
		VERSION "${PROJECT_VERSION}"
		SOVERSION "${PROJECT_VERSION}"
		# allow creating static and shared libs without conflicts
		CLEAN_DIRECT_OUTPUT 1
		# avoid conflicts between library and binary target names
		OUTPUT_NAME hypro
		LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
		ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/src"
		LINKER_LANGUAGE CXX
	)
	#message("hypro libraries dynamic: " ${hypro_LIBRARIES_DYNAMIC})
	#message("hypro include dirs: " ${hypro_INCLUDE_DIRS})
	#message("hypro sources: " ${sources})
	#message("hypro headers: " ${headers})
	target_link_libraries(${PROJECT_NAME} PUBLIC ${hypro_LIBRARIES_DYNAMIC} pthread)
	target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${hypro_INCLUDE_DIRS})
endif()

if(STATICLIB_SWITCH)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		set_target_properties(${PROJECT_NAME}_static PROPERTIES LINKER_LANGUAGE CXX INSTALL_RPATH "@loader_path/../lib")
	else()
		set_target_properties(${PROJECT_NAME}_static PROPERTIES LINKER_LANGUAGE CXX)
	endif()
	set(HYPRO_TARGETS ${PROJECT_NAME}_static CACHE STRING INTERNAL FORCE)
else()
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX INSTALL_RPATH "@loader_path/../lib")
	else()
		set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
	endif()
	set(HYPRO_TARGETS ${PROJECT_NAME} CACHE STRING INTERNAL FORCE)
endif()

# Options for static linking
if (APPLE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mmacosx-version-min=10.7")
else()
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

########################################################################################

cotire(${PROJECT_NAME})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include FILES_MATCHING PATTERN "*.tpp")

########################################################################################

if (CLANG_TIDY)
	add_custom_target(tidy
		COMMAND clang-tidy -p ../../build/compile_commands.json ${CLANG_TIDY_CHECKS} -header-filter='.*' ${sources}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/hypro/
	)
endif()

########################################################################################
