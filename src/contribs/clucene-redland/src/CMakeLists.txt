cmake_policy(SET CMP0003 NEW)

project(clucene-redland-core)

include_directories(
  ${clucene-redland_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  "/home/ben/dev/clucene-rdf/redland-1.0.9/src/"
  "/home/ben/dev/clucene-rdf/redland-1.0.9/gcc/install/include/"
  "/usr/include/rasqal"
  "/usr/lib"
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
