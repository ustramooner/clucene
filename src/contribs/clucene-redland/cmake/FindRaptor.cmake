# - Try to find Raptor
# Once done this will define
#
#  RAPTOR_FOUND       - system has Raptor
#  RAPTOR_LIBRARIES   - Link these to use RAPTOR
#  RAPTOR_DEFINITIONS - Compiler switches required for using RAPTOR
#  RAPTOR_VERSION     - The raptor version string

# (c) 2007-2009 Sebastian Trueg <trueg@kde.org>
#
# Based on FindFontconfig Copyright (c) 2006,2007 Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


INCLUDE(MacroEnsureVersion)


FIND_PROGRAM(
  RAPTOR_CONFIG
  NAMES raptor-config
  )
if(RAPTOR_CONFIG)
  EXECUTE_PROCESS(
    COMMAND ${RAPTOR_CONFIG} --version
    OUTPUT_VARIABLE RAPTOR_VERSION
    )
  if(RAPTOR_VERSION)
    STRING(REPLACE "\n" "" RAPTOR_VERSION ${RAPTOR_VERSION})
    
    MACRO_ENSURE_VERSION("1.4.16" ${RAPTOR_VERSION} RAPTOR_HAVE_TRIG)
    
    # extract include paths from raptor-config
    EXECUTE_PROCESS(
      COMMAND ${RAPTOR_CONFIG} --cflags
      OUTPUT_VARIABLE raptor_CFLAGS_ARGS)
    STRING( REPLACE " " ";" raptor_CFLAGS_ARGS ${raptor_CFLAGS_ARGS} )
    FOREACH( _ARG ${raptor_CFLAGS_ARGS} )
      IF(${_ARG} MATCHES "^-I")
        STRING(REGEX REPLACE "^-I" "" _ARG ${_ARG})
        STRING( REPLACE "\n" "" _ARG ${_ARG} )
        LIST(APPEND raptor_INCLUDE_DIRS ${_ARG})
      ENDIF(${_ARG} MATCHES "^-I")
    ENDFOREACH(_ARG)
    
    # extract lib paths from raptor-config
    EXECUTE_PROCESS(
      COMMAND ${RAPTOR_CONFIG} --libs
      OUTPUT_VARIABLE raptor_CFLAGS_ARGS)
    STRING( REPLACE " " ";" raptor_CFLAGS_ARGS ${raptor_CFLAGS_ARGS} )
    FOREACH( _ARG ${raptor_CFLAGS_ARGS} )
      IF(${_ARG} MATCHES "^-L")
        STRING(REGEX REPLACE "^-L" "" _ARG ${_ARG})
        LIST(APPEND raptor_LIBRARY_DIRS ${_ARG})
      ENDIF(${_ARG} MATCHES "^-L")
    ENDFOREACH(_ARG)
  endif(RAPTOR_VERSION)
else(RAPTOR_CONFIG)
  SET(RAPTOR_VERSION "1.0.0")
endif(RAPTOR_CONFIG)

find_path(RAPTOR_INCLUDE_DIR raptor.h
  PATHS
  ${redland_INCLUDE_DIRS}
  ${raptor_INCLUDE_DIRS}
  /usr/X11/include
  PATH_SUFFIXES redland
  )

find_library(RAPTOR_LIBRARIES NAMES raptor libraptor
  PATHS
  ${raptor_LIBRARY_DIRS}
  )

if (RAPTOR_INCLUDE_DIR AND RAPTOR_LIBRARIES)
  set(RAPTOR_FOUND TRUE)
endif (RAPTOR_INCLUDE_DIR AND RAPTOR_LIBRARIES)

if (RAPTOR_FOUND)
  set(RAPTOR_DEFINITIONS ${raptor_CFLAGS})
  if (NOT Raptor_FIND_QUIETLY)
    message(STATUS "Found Raptor ${RAPTOR_VERSION}: libs - ${RAPTOR_LIBRARIES}; includes - ${RAPTOR_INCLUDE_DIR}")
  endif (NOT Raptor_FIND_QUIETLY)
else (RAPTOR_FOUND)
  if (Raptor_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find Raptor")
  endif (Raptor_FIND_REQUIRED)
endif (RAPTOR_FOUND)

mark_as_advanced(RAPTOR_INCLUDE_DIR_TMP
  RAPTOR_INCLUDE_DIR
  RAPTOR_LIBRARIES)
