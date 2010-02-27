#Locate Boost libs. Windows users: make sure BOOST_ROOT and BOOST_PATH are set correctly on your environment.
#See the site FAQ for more details.

#todo: allow this to fall back on a local distributed copy, so user doesn't have to d/l Boost seperately
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE( Boost )
#todo: limit Boost version?
#todo: use COMPONENTS threads to locate boost_threads without breaking the current support
IF(Boost_FOUND)
  IF (NOT _boost_IN_CACHE)
    MESSAGE( "Boost found" )
    message(STATUS "Boost_INCLUDE_DIR    : ${Boost_INCLUDE_DIR}")
  ENDIF (NOT _boost_IN_CACHE)
	INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )
ELSE()
    MESSAGE( "Boost not found, using local" )
    INCLUDE_DIRECTORIES( ${clucene-ext_SOURCE_DIR}/ )
ENDIF()


