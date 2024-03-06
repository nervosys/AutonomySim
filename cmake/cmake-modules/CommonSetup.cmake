# Common setup instructions shared by all AutonomySim CMakeLists

macro(CommonTargetLink)
    # Target_link_libraries(c++abi)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
endmacro(CommonTargetLink)

macro(IncludeEigen)
    include_directories(${AUTONOMYSIM_ROOT}/AutonomyLib/deps/eigen3)
endmacro(IncludeEigen)

macro(AddExecutableSource)
    set(PROJECT_CPP ${PROJECT_NAME}_sources)
    file(GLOB_RECURSE PROJECT_CPP "${AUTONOMYSIM_ROOT}/${PROJECT_NAME}/*.cpp")
    add_executable(${PROJECT_NAME} ${PROJECT_CPP})
endmacro(AddExecutableSource)

macro(SetupConsoleBuild)
    if(UNIX)
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CONSOLE ")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
    endif()
endmacro(SetupConsoleBuild)

macro(CommonSetup)
    find_package(Threads REQUIRED)
    find_path(AUTONOMYSIM_ROOT NAMES "AutonomySim.sln" PATHS ".." "../.." "../../.." "../../../.." "../../../../.." "../../../../../.." REQUIRED)

    # Setup output paths
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/output/bin)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

    # Setup include and lib for rpclib which will be referenced by other projects
    set(RPCLIB_VERSION "2.3.0")
    set(RPC_LIB_INCLUDES " ${AUTONOMYSIM_ROOT}/external/rpclib/rpclib-${RPCLIB_VERSION}/include")
    #name of .a file with lib prefix
    set(RPC_LIB "rpc")

    # What is our build type debug or release?
    string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)

    if(UNIX)
        set(RPC_LIB_DEFINES "-D MSGPACK_PP_VARIADICS_MSVC=0")
        set(BUILD_TYPE "linux")
        set(CMAKE_CXX_STANDARD 17)

        if(APPLE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wstrict-aliasing -D__CLANG__")
        else()
            set(CMAKE_CXX_FLAGS "\
                -Wall -Wextra \
                -Wnon-virtual-dtor -Woverloaded-virtual \
                -Wno-variadic-macros -Wno-unused-function -Wno-unused \
                -pthread \
                ${RPC_LIB_DEFINES} ${CMAKE_CXX_FLAGS}")

            if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
                set(CMAKE_CXX_FLAGS "-stdlib=libc++ -Wno-documentation -Wno-unknown-warning-option ${CMAKE_CXX_FLAGS}")
                find_package(LLVM REQUIRED CONFIG)
                set(CXX_EXP_LIB "-L${LLVM_LIBRARY_DIRS} -lc++fs -ferror-limit=10")
            else()
                set(CXX_EXP_LIB "-lstdc++fs -fmax-errors=10 -Wnoexcept -Wstrict-null-sentinel")
            endif()
        endif()

        set(BUILD_PLATFORM "x64")
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
        if(CMAKE_BUILD_TYPE MATCHES "Release")
            set(CMAKE_CXX_FLAGS "-O3 ${CMAKE_CXX_FLAGS}")
        endif()
    else()
        # WARNING: Windows CMake build is experimental
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0600 /GS /W4 /wd4100 /wd4505 /wd4820 /wd4464 /wd4514 /wd4710 /wd4571 /Zc:wchar_t /ZI /Zc:inline /fp:precise /D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS /D_UNICODE /DUNICODE /WX- /Zc:forScope /Gd /EHsc ")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NXCOMPAT /DYNAMICBASE /INCREMENTAL:NO ")

        if("${BUILD_TYPE}" STREQUAL "debug")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_DEBUG /MDd /RTC1 /Gm /Od ")
        elseif("${BUILD_TYPE}" STREQUAL "release")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD /O2 /Oi /GL /Gm- /Gy /TP ")
        else()
          message(FATAL_ERROR "Please specify '-D CMAKE_BUILD_TYPE=Debug' or 'Release' on the cmake command line")
        endif()
    endif()
    
    # TODO: We are not using Boost anymore so the below should not be needed.
    # Use common boost settings for consistency.
    #set(Boost_USE_STATIC_LIBS ON)
    #set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_RUNTIME ON)

    ## TODO: probably should set x64 explicitly
    ## strip x64 from /machine:x64 from CMAKE_STATIC_LINKER_FLAGS and set in BUILD_PLATFORM
    if(NOT "${CMAKE_STATIC_LINKER_FLAGS}" STREQUAL "")
      string(SUBSTRING ${CMAKE_STATIC_LINKER_FLAGS} 9 -1 "BUILD_PLATFORM")
    endif()

endmacro(CommonSetup)
