set(glpk_version "4.45")
ExternalProject_Add(
	glpk
	#URL ftp://ftp.gnu.org/gnu/glpk/glpk-${glpk_version}.tar.gz
	SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/resources/glpk-${glpk_version}"
    CONFIGURE_COMMAND ./configure --prefix=${CMAKE_BINARY_DIR}/resources/glpk-${glpk_version}/ --with-gmp
	PREFIX ${CMAKE_BINARY_DIR}/resources/glpk-${glpk_version}/
            #BUILD_COMMAND make
            BUILD_IN_SOURCE 1
            INSTALL_DIR ${CMAKE_BINARY_DIR}/resources/glpk-${glpk_version}/
            INSTALL_COMMAND make install
	)

ExternalProject_Get_Property(glpk source_dir)
ExternalProject_Get_Property(glpk binary_dir)

#add_imported_library(GLPK SHARED "${CMAKE_BINARY_DIR}/resources/glpk-${glpk_version}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}glpk${CMAKE_SHARED_LIBRARY_SUFFIX}" "${CMAKE_BINARY_DIR}/resources/glpk-${glpk_version}/include")

set( GLPK_INCLUDE_DIR "${CMAKE_BINARY_DIR}/resources/glpk-${glpk_version}/include" PARENT_SCOPE )
set( GLPK_LIBRARIES "${CMAKE_BINARY_DIR}/resources/glpk-${glpk_version}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}glpk${CMAKE_SHARED_LIBRARY_SUFFIX}" PARENT_SCOPE )

add_library(GLPK_SHARED SHARED IMPORTED)
set_target_properties(GLPK_SHARED PROPERTIES IMPORTED_LOCATION "${GLPK_LIBRARIES}")
set_target_properties(GLPK_SHARED PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLPK_INCLUDE_DIR}")
set(hypro_LIBRARIES_DYNAMIC ${hypro_LIBRARIES_DYNAMIC} ${GLPK_LIBRARIES} )

add_dependencies(resources glpk)
