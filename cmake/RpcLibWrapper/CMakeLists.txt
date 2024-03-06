cmake_minimum_required(VERSION 3.5.0)
project(RpcLibWrapper)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake-modules") 
include("${CMAKE_CURRENT_LIST_DIR}/../cmake-modules/CommonSetup.cmake")

CommonSetup()

if(UNIX)
    set(RPC_LIB_DEFINES " -D MSGPACK_PP_VARIADICS_MSVC=0")
    set(CMAKE_CXX_FLAGS "${RPC_LIB_DEFINES} ${CMAKE_CXX_FLAGS}")    
endif()

add_subdirectory("${AUTONOMYSIM_ROOT}/external/rpclib/$rpclib-{RPCLIB_VERSION}" "${CMAKE_CURRENT_BINARY_DIR}/rpclib")
