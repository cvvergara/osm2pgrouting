PROJECT(getnodes)
CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
  message(FATAL_ERROR "In-source builds not allowed.
  Please make a new directory (called a build directory) and run CMake from there.
  You may need to remove CMakeCache.txt." )
endif()

# Set language to use: C++
enable_language(CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(SHARE_DIR "/usr/share/getnodes")

set(CMAKE_CXX_STANDARD 17)

#-----------------------------------------------------
# Dependencies
#-----------------------------------------------------

find_package(PostgreSQL REQUIRED)
if(POSTGRESQL_FOUND)
  include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR})
else()
  message(FATAL_ERROR "PostgreSQL not found!\n")
endif()

find_package(libpqxx REQUIRED)
if(LibPQX_FOUND)
  include_directories(SYSTEM ${LIBPQXX_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "LibPQXX not found!\n")
endif()

find_package(EXPAT REQUIRED)
if(EXPAT_FOUND)
  include_directories(SYSTEM ${EXPAT_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "expat not found!\n")
endif()

find_package(Osmium REQUIRED COMPONENTS io pbf xml)
if(OSMIUM_FOUND)
  include_directories(SYSTEM ${OSMIUM_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "Libosmium not found!\n")
endif()

find_package(Boost COMPONENTS program_options REQUIRED)
if(Boost_INCLUDE_DIRS)
  include_directories(${Boost_INCLUDE_DIR})
  message(STATUS "Boost headers were found here: ${Boost_INCLUDE_DIRS}")
else()
  message(FATAL_ERROR " Please check your Boost installation ")
endif()


FILE(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp"  "${CMAKE_SOURCE_DIR}/src/*/*.cpp")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64 -std=c++0x")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -Wconversion -pedantic -Wextra  -frounding-math -Wno-deprecated -fmax-errors=10")


set (GETNODE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include")
message(STATUS "LIBPQXX_INCLUDE_DIRS: ${LIBPQXX_INCLUDE_DIRS}")
message(STATUS "POSTGRESQL_INCLUDE_DIR: ${POSTGRESQL_INCLUDE_DIR}")
message(STATUS "EXPAT_INCLUDE_DIRS: ${EXPAT_INCLUDE_DIRS}")
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "POSTGRESQL_LIBRARIES: ${POSTGRESQL_LIBRARIES}")
message(STATUS "Boost_LIBRARIES: ${boost_LIBRARIES}")
message(STATUS "LIBPQXX_LIBRARIES: ${LIBPQXX_LIBRARIES}")

INCLUDE_DIRECTORIES(src
  ${GETNODE_INCLUDE_DIRS}
  )

ADD_EXECUTABLE(getnodes ${SOURCES})

TARGET_LINK_LIBRARIES(getnodes
  ${LIBPQXX_LIBRARIES}
  ${POSTGRESQL_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OSMIUM_LIBRARIES}
  )

INSTALL(TARGETS getnodes
  RUNTIME DESTINATION "/usr/bin"
  )

