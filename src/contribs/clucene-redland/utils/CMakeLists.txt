project(clucene-redland-tests)

include_directories(
  ${clucene-redland_SOURCE_DIR}
  ${REDLAND_INCLUDE_DIR}
  ${RASQAL_INCLUDE_DIR}
  ${RAPTOR_INCLUDE_DIR}
)

add_executable(cl_rdfprocpp rdfprocpp.cpp)
target_link_libraries(cl_rdfprocpp clucene-redland ${CLUCENE_LIBRARY} ${REDLAND_LIBRARIES} )

