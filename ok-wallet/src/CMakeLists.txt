################################################################################
# copy from raiblocks/CMakeLists.txt

set (RAIBLOCKS_SECURE_RPC OFF CACHE BOOL "")
option(RAIBLOCKS_ASAN "Enable ASan+UBSan" OFF)
option(RAIBLOCKS_ASAN_INT "Enable ASan+UBSan+Integer overflow" OFF)
option(RAIBLOCKS_SIMD_OPTIMIZATIONS "Enable CPU-specific SIMD optimizations (SSE/AVX or NEON, e.g.)" OFF)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)

find_package (Boost 1.66.0 REQUIRED COMPONENTS date_time filesystem system log log_setup thread program_options regex chrono atomic)
include_directories (${Boost_INCLUDE_DIR})

if (RAIBLOCKS_SECURE_RPC)
	set (OPENSSL_LIBRARIES $ENV{COIN_DEPS}/openssl/lib/libcrypto.a $ENV{COIN_DEPS}/openssl/lib/libssl.a)
	include_directories($ENV{COIN_DEPS}/openssl/include)
else ()
	set (OPENSSL_LIBRARIES "")
endif (RAIBLOCKS_SECURE_RPC)

if (WIN32)
	set (PLATFORM_LIBS Ws2_32 mswsock iphlpapi ntdll)
else (WIN32)
	set (PLATFORM_LIBS pthread ${CMAKE_DL_LIBS})
endif (WIN32)

if (APPLE)
	set (PLATFORM_LINK_FLAGS "-framework Foundation -framework OpenCL")
elseif (WIN32)
	set (PLATFORM_LINK_FLAGS "")
else ()
	set (PLATFORM_LINK_FLAGS "-static-libgcc -static-libstdc++")
	if (RAIBLOCKS_ASAN)
		set (PLATFORM_LINK_FLAGS "${PLATFORM_LINK_FLAGS} -fsanitize=address,undefined")
	elseif (RAIBLOCKS_ASAN_INT)
		set (PLATFORM_LINK_FLAGS "${PLATFORM_LINK_FLAGS} -fsanitize=address,undefined,integer")
	elseif (RAIBLOCKS_TSAN)
		set (PLATFORM_LINK_FLAGS "${PLATFORM_LINK_FLAGS} -fsanitize=thread")
	endif()
endif (APPLE)

if (CRYPTOPP_CUSTOM OR NOT RAIBLOCKS_SIMD_OPTIMIZATIONS)
	set (CRYPTOPP_LIBPATH ${PROJECT_SOURCE_DIR}/..)
	set (CRYPTOPP_LIBRARY cryptopp)
else ()
	set (CRYPTOPP_LIBPATH ${PROJECT_SOURCE_DIR}/../cryptopp)
	set (CRYPTOPP_LIBRARY cryptopp-static)
endif ()

################################################################################
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

set(RAIBLOCKS_SOURCE
	${PROJECT_SOURCE_DIR}/../rai/rai_node/daemon.cpp
)

set(WALLET_LIB_SOURCE
  wallet.cpp
	app_wrapper.cpp
	test_helper.cpp
	com_okcoin_vault_jni_raiblocks_XrbJ.cpp
)


include_directories(${PROJECT_SOURCE_DIR}/..)
include_directories(${PROJECT_SOURCE_DIR}/../miniupnp/miniupnpc)
include_directories($ENV{JAVA_HOME}/include)
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  include_directories($ENV{JAVA_HOME}/include/darwin)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  include_directories($ENV{JAVA_HOME}/include/linux)
endif()

add_library(${OKLIBRARY_NAME} SHARED ${RAIBLOCKS_SOURCE} ${WALLET_LIB_SOURCE})
add_dependencies(${OKLIBRARY_NAME} raiblocks-dep)
target_link_libraries(${OKLIBRARY_NAME} ${PROJECT_SOURCE_DIR}/../libnode.a ${PROJECT_SOURCE_DIR}/../libsecure.a ${PROJECT_SOURCE_DIR}/../liblmdb.a ${PROJECT_SOURCE_DIR}/../libed25519.a ${PROJECT_SOURCE_DIR}/../librai_lib_static.a ${PROJECT_SOURCE_DIR}/../libargon2.a ${PROJECT_SOURCE_DIR}/../libblake2.a ${PROJECT_SOURCE_DIR}/../libxxhash.a ${OPENSSL_LIBRARIES} ${CRYPTOPP_LIBPATH}/lib${CRYPTOPP_LIBRARY}.a ${PROJECT_SOURCE_DIR}/../miniupnp/miniupnpc/libminiupnpc.a ${Boost_ATOMIC_LIBRARY} ${Boost_CHRONO_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_LOG_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_LOG_SETUP_LIBRARY} ${Boost_THREAD_LIBRARY} ${PLATFORM_LIBS})
set_target_properties (${OKLIBRARY_NAME} PROPERTIES LINK_FLAGS "${PLATFORM_LINK_FLAGS}")

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
