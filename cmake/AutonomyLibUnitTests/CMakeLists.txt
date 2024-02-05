cmake_minimum_required(VERSION 3.5.0)
project(AutonomyLibUnitTests)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake-modules") 
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/../cmake-modules/CommonSetup.cmake")
CommonSetup()

IncludeEigen()

SetupConsoleBuild()

include_directories(
  ${AutonomySim_ROOT}/AutonomyLibUnitTests
  ${AutonomySim_ROOT}/AutonomyLib/include
  ${AutonomySim_ROOT}/MavLinkCom/include
)

AddExecutableSource()

CommonTargetLink()
target_link_libraries(${PROJECT_NAME} AutonomyLib)
target_link_libraries(${PROJECT_NAME} MavLinkCom)
target_link_libraries(${PROJECT_NAME} ${RPC_LIB})
