ExternalProject_Add (proj_raiblocks
	PREFIX ${PROJECT_SOURCE_DIR}/..
	SOURCE_DIR ${PROJECT_SOURCE_DIR}/..
	INSTALL_DIR ${PROJECT_BINARY_DIR}/deps-install
	TMP_DIR ${PROJECT_BINARY_DIR}/deps-tmp/raiblocks
	STAMP_DIR ${PROJECT_BINARY_DIR}/deps-stamp/raiblocks
	CMAKE_ARGS -DBOOST_ROOT=${BOOST_ROOT} -DACTIVE_NETWORK=${ACTIVE_NETWORK}
	INSTALL_COMMAND ""
	BUILD_IN_SOURCE 1
)
ExternalProject_Get_Property(proj_raiblocks source_dir)
add_library(raiblocks-dep STATIC IMPORTED)
set_target_properties(raiblocks-dep PROPERTIES
	IMPORTED_LOCATION ${source_dir}/rai_node
)
add_dependencies(raiblocks-dep proj_raiblocks)

################################################################################

set(STELLAR_CORE_SOURCE
)

set(WALLET_LIB_SOURCE
  wallet.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/..)
include_directories($ENV{JAVA_HOME}/include)
include_directories($ENV{JAVA_HOME}/include/darwin)

add_library(raiblocks SHARED ${STELLAR_CORE_SOURCE} ${WALLET_LIB_SOURCE})
add_dependencies(raiblocks raiblocks-dep)

################################################################################
ExternalProject_Add (proj_gtest
	PREFIX ${PROJECT_SOURCE_DIR}/test/deps/googletest
	SOURCE_DIR ${PROJECT_SOURCE_DIR}/test/deps/googletest
	INSTALL_DIR ${PROJECT_BINARY_DIR}/deps-install
	TMP_DIR ${PROJECT_BINARY_DIR}/deps-tmp/googletest
	STAMP_DIR ${PROJECT_BINARY_DIR}/deps-stamp/googletest
	BINARY_DIR ${PROJECT_BINARY_DIR}/deps-binary/googletest
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)
ExternalProject_Get_Property(proj_gtest source_dir)
ExternalProject_Get_Property(proj_gtest install_dir)
add_library(gtest STATIC IMPORTED)
set_target_properties(gtest PROPERTIES
	IMPORTED_LOCATION ${install_dir}/lib/libgmock_main.a
)
add_dependencies(gtest proj_gtest)

include_directories(${install_dir}/include)
include_directories(${install_dir}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

enable_testing()
include(add_test.cmake)
cxx_test(test_okwallet "" gtest)
