cmake_minimum_required(VERSION 3.5.0)
project(MavLinkCom)

add_subdirectory("MavLinkTest")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake-modules") 
include("${CMAKE_CURRENT_LIST_DIR}/../cmake-modules/CommonSetup.cmake")

CommonSetup()

include_directories(
  ${AUTONOMYSIM_ROOT}/MavLinkCom
  ${AUTONOMYSIM_ROOT}/MavLinkCom/common_utils
  ${AUTONOMYSIM_ROOT}/MavLinkCom/include
)

list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/common/utils/FileSystem.cpp")
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/common/utils/ThreadUtils.cpp")
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/AdHocConnection.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/MavLinkConnection.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/MavLinkFtpClient.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/MavLinkLog.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/MavLinkMessageBase.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/MavLinkMessages.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/MavLinkNode.cpp") 	
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/MavLinkTcpServer.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/MavLinkVehicle.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/MavLinkVideoStream.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/Semaphore.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/UdpSocket.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/impl/AdHocConnectionImpl.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/impl/MavLinkConnectionImpl.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/impl/MavLinkFtpClientImpl.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/impl/MavLinkNodeImpl.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/impl/MavLinkTcpServerImpl.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/impl/MavLinkVehicleImpl.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/impl/MavLinkVideoStreamImpl.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/impl/UdpSocketImpl.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/serial_com/SerialPort.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/serial_com/TcpClientPort.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/serial_com/UdpClientPort.cpp") 
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/serial_com/SocketInit.cpp")
list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/serial_com/wifi.cpp")

if(UNIX)
  list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/impl/linux/MavLinkFindSerialPorts.cpp")
else()
  list(APPEND MAVLINK_SOURCES "${AUTONOMYSIM_ROOT}/MavLinkCom/src/impl/windows/WindowsFindSerialPorts.cpp")
endif()

add_library(MavLinkCom STATIC ${MAVLINK_SOURCES})

CommonTargetLink()
