CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

PROJECT(getgraph)

include(CMakePrintHelpers)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

SET(SHARE_DIR "/usr/share/getgraph")

# All files under src are compiled
FILE(GLOB SRC "${CMAKE_SOURCE_DIR}/src/*.cpp"  "${CMAKE_SOURCE_DIR}/src/*/*.cpp")
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(PostgreSQL REQUIRED)
include_directories(SYSTEM ${PostgreSQL_INCLUDE_DIR})

find_package(EXPAT REQUIRED)
include_directories(SYSTEM ${EXPAT_INCLUDE_DIRS})

find_package(Osmium REQUIRED COMPONENTS io pbf xml)
include_directories(SYSTEM ${OSMIUM_INCLUDE_DIRS})

# -------------
# LibPQXX
# -------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

if (PQXX_VERSION VERSION_GREATER_EQUAL "7.0.0")
  set(CMAKE_CXX_STANDARD 17)
else()
  add_definitions(-DPQXX_DISCONNECT)
  set(CMAKE_CXX_STANDARD 14)
endif()
include_directories(SYSTEM ${PQXX_INCLUDE_DIRS})


find_package(Boost COMPONENTS program_options REQUIRED)
if(Boost_INCLUDE_DIRS)
    message(STATUS "Boost headers were found here: ${Boost_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR " Please check your Boost installation ")
endif()
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

cmake_print_variables(PostgreSQL_INCLUDE_DIR PostgreSQL_LIBRARIES)
cmake_print_variables(EXPAT_INCLUDE_DIRS EXPAT_LIBRARIES)
Cmake_print_variables(OSMIUM_INCLUDE_DIRS OSMIUM_LIBRARIES)
cmake_print_variables(PQXX_VERSION PQXX_INCLUDE_DIRS)
cmake_print_variables(Boost_INCLUDE_DIRS Boost_LIBRARIES)


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -Wconversion -pedantic -Wextra  -frounding-math -Wno-deprecated -fmax-errors=10")

#---------------------------------------------
# Clang tidy
#---------------------------------------------

option(USE_CLANG_TIDY "Use clang-tidy." OFF)

if(USE_CLANG_TIDY)
    find_program(CLANG_TIDY_COMMAND NAMES clang-tidy NO_CACHE)
    if (CLANG_TIDY_COMMAND)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND};--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
    else()
        message(WARNING "Set to use clang-tidy, but not found.")
    endif()
endif()

cmake_print_variables(CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG)

add_executable(getgraph ${SRC})

TARGET_LINK_LIBRARIES(getgraph
    ${PostgreSQL_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OSMIUM_LIBRARIES}
)

INSTALL(TARGETS getgraph
    RUNTIME DESTINATION "/usr/bin"
)
