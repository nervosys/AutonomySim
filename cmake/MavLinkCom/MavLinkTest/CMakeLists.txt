cmake_minimum_required(VERSION 3.2.2)
project(MavLinkTest)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake-modules") 
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake-modules/CommonSetup.cmake")
CommonSetup()

SetupConsoleBuild()

include_directories(
  ${AUTONOMYSIM_ROOT}/MavLinkCom/MavLinkTest
  ${AUTONOMYSIM_ROOT}/MavLinkCom/include
  ${AUTONOMYSIM_ROOT}/MavLinkCom/common_utils
)

set(PROJECT_CPP ${PROJECT_NAME}_sources)
file(GLOB_RECURSE PROJECT_CPP "${AUTONOMYSIM_ROOT}/MavLinkCom/${PROJECT_NAME}/*.cpp")
add_executable(${PROJECT_NAME} ${PROJECT_CPP})
			
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} MavLinkCom ${CXX_EXP_LIB})
