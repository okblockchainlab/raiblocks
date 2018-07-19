cmake_minimum_required(VERSION 3.4)
project(raiblocks-wallet)

if(${PROJECT_BINARY_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
	message(FATAL_ERROR "DO NOT run cmake in source directory")
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if (NOT OKLIBRARY_NAME)
	set(OKLIBRARY_NAME raiblocks)
endif()

if (NOT BOOST_ROOT)
	message(FATAL_ERROR "undefined BOOST_ROOT")
endif()

SET (ACTIVE_NETWORK rai_live_network CACHE STRING "Selects which network parameters are used")
set_property (CACHE ACTIVE_NETWORK PROPERTY STRINGS rai_test_network rai_beta_network rai_live_network)
if (NOT ACTIVE_NETWORK)
	set(ACTIVE_NETWORK "rai_live_network")
endif()
add_definitions(-DACTIVE_NETWORK=${ACTIVE_NETWORK})

set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

enable_testing()

link_libraries(pthread)
link_libraries(dl)

include(ExternalProject)
add_subdirectory(src)
