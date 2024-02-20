cmake_minimum_required(VERSION 3.5.0)
project(DroneShell)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake-modules") 
include("${CMAKE_CURRENT_LIST_DIR}/../cmake-modules/CommonSetup.cmake")

CommonSetup()
IncludeEigen()
SetupConsoleBuild()

include_directories(
  ${AUTONOMYSIM_ROOT}/DroneShell
  ${RPC_LIB_INCLUDES}
  ${AUTONOMYSIM_ROOT}/DroneShell/include;
  ${AUTONOMYSIM_ROOT}/AutonomyLib/include
  ${AUTONOMYSIM_ROOT}/MavLinkCom/include
)

AddExecutableSource()
CommonTargetLink()

target_link_libraries(${PROJECT_NAME} AutonomyLib)
target_link_libraries(${PROJECT_NAME} ${RPC_LIB})
