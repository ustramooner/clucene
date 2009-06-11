cmake_policy(SET CMP0003 NEW)

project(clucene-redland-core)

include_directories(
  ${clucene-redland_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CLUCENE_INCLUDE_DIR}
  ${CLUCENE_LIBRARY_DIR}
  ${REDLAND_INCLUDE_DIR}
  ${RASQAL_INCLUDE_DIR}
  ${RAPTOR_INCLUDE_DIR}
)

set( CLUCNENE_REDLAND_CORE_SRCS
  CLuceneStorage.cpp
  rdf_storage_clucene.cpp
)
#find our headers
file(GLOB_RECURSE CLUCNENE_REDLAND_CORE_HEADERS ${clucene-redland_SOURCE_DIR}/redlandpp/*.hpp ${clucene-redland_SOURCE_DIR}/redlandpp/*.h)

add_library(clucene-redland SHARED ${CLUCNENE_REDLAND_CORE_SRCS} ${CLUCNENE_REDLAND_CORE_HEADERS})

set_target_properties(
  clucene-redland
  PROPERTIES
  VERSION 0.1
  SOVERSION 1
  DEFINE_SYMBOL MAKE_CLUCENE_REDLAND_LIB
  INSTALL_NAME_DIR ${LIB_DESTINATION}
)

INSTALL(TARGETS clucene-redland
	LIBRARY DESTINATION ${LIB_DESTINATION}
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
)
