cmake_minimum_required(VERSION 3.10.2)

project(Smart_Home_Hub)

## Default directories ##
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(XBEE_INLCUDES "/usr/local/include/xbee")
ADD_DEFINITIONS("-DPOSIX")

# In case of GNU CPP compiler - check version before
if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
    message(FATAL_ERROR "Radio Application source code compilation requires at least gcc-4.8")
endif()

# pthreads
find_package(Threads REQUIRED)

# openzwave library
find_library(OPENZWAVE_LIB PATH_SUFFIXES lib lib64 NAMES libopenzwave.so libopenzwave.dylib)
if(NOT OPENZWAVE_LIB)
  message(FATAL_ERROR "open_zwave library not found.")
endif()

# openzwave headers
find_path(OPENZWAVE_INCLUDES openzwave/Manager.h)
if(NOT OPENZWAVE_INCLUDES)
  message(FATAL_ERROR "open_zwave library headers not found")
else(OPENZWAVE_INCLUDES)
  message(OPENZWAVE_INCLUDES= ${OPENZWAVE_INCLUDES})
endif()


#Digi Xbee library
find_library(DIGIXBEE_LIB PATH_SUFFIXES lib lib64 NAMES libdigixbee.so)
if(NOT DIGIXBEE_LIB)
	message(FATAL_ERROR "Xbee library not found.")
endif()

# Digi XBee headers
find_path(XBEE_INLCUDES xbee/xbee.h)
if(NOT XBEE_INLCUDES)
  message(FATAL_ERROR "Digi Xbee library headers not found")
else(XBEE_INLCUDES)
  message(XBEE_INLCUDES= ${XBEE_INLCUDES})
endif()


find_package(PkgConfig REQUIRED)



# jsoncpp library
find_library(JSONCPP_LIB PATH_SUFFIXES lib lib64 NAMES libjsoncpp.so)
if(NOT JSONCPP_LIB)
  message(FATAL_ERROR "jsoncpp library not found.")
else(JSONCPP_LIB)
  message("INFO: we found LibJsoncpp on your pc.")
    message(JSONCPP_LIB = ${JSONCPP_LIB})
    message(Jsoncpp_INCLUDE_DIR = ${Jsoncpp_INCLUDE_DIR}) 
    message(Jsoncpp_LIBRARY = ${Jsoncpp_LIBRARY})
endif()

  
# jsoncpp headers
find_path(JSONCPP_INCLUDES json/json.h)
if(NOT JSONCPP_INCLUDES)
  message(FATAL_ERROR "jsoncpp library headers not found")
endif()

# valgrind
find_program(VALGRIND valgrind)
if(NOT VALGRIND)
    message(STATUS "Valgrind DISABLED: not installed")
else()
    message(STATUS "Valgrind ENABLED. (${VALGRIND})")
endif()

# global include directories
include_directories(${SRC_DIR})
include_directories(${OPENZWAVE_INCLUDES} ${OPENZWAVE_INCLUDES}/openzwave)
include_directories(${XBEE_INLCUDES} )#${OPENZWAVE_INCLUDES}/openzwave)
include_directories(${JSONCPP_INCLUDES})
include_directories(${SRC_DIR})
include_directories(${XBEE_INLCUDES})
add_subdirectory(${SRC_DIR})

message(STATUS "Source Directory        :  ${SRC_DIR}")
message(STATUS "C++ compiler            :  ${CMAKE_CXX_COMPILER}")
message(STATUS "OpenZwave Includes      :  ${OPENZWAVE_INCLUDES}/openzwave/")
message(STATUS "OpenZwave Libraries     :  ${OPENZWAVE_LIB}")
message(STATUS "Xbee Includes      		:  ${XBEE_INLCUDES}/xbee/")
message(STATUS "Xbee Libraries     		:  ${DIGIXBEE_LIB}")

message(STATUS "JSONCPP Includes        :  ${JSONCPP_INCLUDES}/json/")
message(STATUS "JSON CPP Libraries      :  ${JSONCPP_LIB}")




