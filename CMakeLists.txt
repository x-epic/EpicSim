cmake_minimum_required(VERSION 3.4.3)
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to conform to")
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)
set(CMAKE_C_STANDARD 11)
project(EpicSim)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)
set(CMAKE_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif()
string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
if (CMAKE_BUILD_TYPE AND
    NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE)$")
  message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

# coverage option
OPTION (ENABLE_COVERAGE "Use gcov" OFF)
MESSAGE(STATUS ENABLE_COVERAGE=${ENABLE_COVERAGE})
IF(ENABLE_COVERAGE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()

# find dependency compile tools
# gperf
# bison
# flex
find_program(GPERF gperf)
if (GPERF-NOTFOUND)
  message(FATAL_ERROR "please install gperf!")
endif()
find_program(BISON bison)
if (BISON-NOTFOUND)
  message(FATAL_ERROR "please install bison!")
endif()
find_program(FLEX flex)
if (FLEX-NOTFOUND)
  message(FATAL_ERROR "please install flex!")
endif()

set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/package) 

add_subdirectory(ivl/)
add_subdirectory(ivlpp/)
add_subdirectory(driver/)
add_subdirectory(libveriuser/)
add_subdirectory(tgt-vvp/)
add_subdirectory(vhdlpp/)
add_subdirectory(vpi/)
add_subdirectory(vvp/)
