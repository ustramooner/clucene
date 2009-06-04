project(clucene-redland-tests)

include_directories(
  ${clucene-redland_SOURCE_DIR}
  "/home/ben/dev/clucene-rdf/redland-1.0.9/gcc/install/include/"
  "/home/ben/dev/clucene-rdf/redlandpp/gcc/install/include"
  /usr/include/rasqal
)

add_executable(cl_rdfprocpp rdfprocpp.cpp)
target_link_libraries(cl_rdfprocpp clucene-redland /home/ben/dev/clucene-rdf/redlandpp/gcc/install/lib/libredlandpp.so clucene /home/ben/dev/clucene-rdf/redland-1.0.9/gcc/install/lib/librdf.so )

add_executable(cl_rdfprocpp2 rdfprocpp2.cpp)
target_link_libraries(cl_rdfprocpp2 clucene-redland /home/ben/dev/clucene-rdf/redlandpp/gcc/install/lib/libredlandpp.so clucene /home/ben/dev/clucene-rdf/redland-1.0.9/gcc/install/lib/librdf.so )
