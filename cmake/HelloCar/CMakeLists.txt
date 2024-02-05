cmake_minimum_required(VERSION 3.5.0)
project(HelloCar)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake-modules") 
include("${CMAKE_CURRENT_LIST_DIR}/../cmake-modules/CommonSetup.cmake")
CommonSetup()

IncludeEigen()

SetupConsoleBuild()

## Specify additional locations of header files
include_directories(
  ${AutonomySim_ROOT}/HelloCar
  ${AutonomySim_ROOT}/AutonomyLib/include
  ${RPC_LIB_INCLUDES}
  ${AutonomySim_ROOT}/MavLinkCom/include
  ${AutonomySim_ROOT}/MavLinkCom/common_utils
)

AddExecutableSource()
			
CommonTargetLink()
target_link_libraries(${PROJECT_NAME} AutonomyLib)
target_link_libraries(${PROJECT_NAME} ${RPC_LIB})
