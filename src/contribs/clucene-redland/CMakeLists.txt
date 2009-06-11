project(clucene-redland)

cmake_minimum_required(VERSION 2.6.2)
INCLUDE ( CheckIncludeFileCXX )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake) 


##################  setup install directories  ################################
set (LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set (LIB_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Library directory name")
## the following are directories where stuff will be installed to
set(INCLUDE_INSTALL_DIR      "${CMAKE_INSTALL_PREFIX}/include/" CACHE PATH "The subdirectory to the header prefix")

# Set up RPATH handling, so the libs are found if they are installed to a non-standard location.
# By default cmake builds the targets with full RPATH to everything in the build directory,
# but then removes the RPATH when installing.
# These two options below make it set the RPATH of the installed targets to all
# RPATH directories outside the current CMAKE_BINARY_DIR and also the library 
# install directory. Alex
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH  TRUE)
set(CMAKE_INSTALL_RPATH                ${LIB_DESTINATION} )

if(APPLE)
   set(CMAKE_INSTALL_NAME_DIR ${LIB_DESTINATION})
endif(APPLE)


##################  find packages  ################################

find_package(Rasqal)
find_package(Raptor)
find_package(Redland)
find_package(CLucene)

##################  some compiler settings ################################
if(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32 )
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wformat-security -fno-check-new -fno-common")
endif(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32 )
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -Zc:wchar_t-)
endif(MSVC)
if(MINGW)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -Wpointer-arith -Wformat-security -fno-check-new -fno-common")
endif(MINGW)


##################  add subdirectories ################################
add_subdirectory(src) 
add_subdirectory(utils)
