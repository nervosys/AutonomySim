cmake_minimum_required(VERSION 3.5.0)
project(HelloSpawnedDrones)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake-modules") 
include("${CMAKE_CURRENT_LIST_DIR}/../cmake-modules/CommonSetup.cmake")

CommonSetup()
IncludeEigen()
SetupConsoleBuild()

## Specify additional locations of header files
include_directories(
  ${AUTONOMYSIM_ROOT}/HelloSpawnedDrones
  ${AUTONOMYSIM_ROOT}/AutonomyLib/include
  ${RPC_LIB_INCLUDES}
  ${AUTONOMYSIM_ROOT}/MavLinkCom/include
  ${AUTONOMYSIM_ROOT}/MavLinkCom/common/utils
)

AddExecutableSource()			
CommonTargetLink()

target_link_libraries(${PROJECT_NAME} AutonomyLib)
target_link_libraries(${PROJECT_NAME} ${RPC_LIB})
