#!/bin/bash
#----------------------------------------------------------------------------------------
# Filename
#   build.sh
# Description
#   BASH script to build AutonomySim plugin for Unreal Engine.
# Authors
#   Microsoft (original)
#   Adam Erickson (Nervosys)
# Date
#   2024-02-20
# Usage
#   `bash build.sh --gcc`
# Notes
# - This script is a cleaned up version of the original AirSim script.
# TODO
# - Update: ensure script runs on recent Linux distributions, MacOS, Windows.
#----------------------------------------------------------------------------------------

set -x  # print shell commands before executing (for debugging)
set -e  # exit on error return code

###
### Functions
###

# System information.
function system_info {
    uname -a && lscpu
}

# System architecture.
function system_arch {
    # unreliable: uname -m
    lscpu | grep 'Architecture' | awk {'print $2'}
}

# System OS version.
function system_os {
    uname -r
}

###
### Variables
###

# Static variables.
DEBUG='false'
USE_GCC='false'

CMAKE_VERSION='3.29.2'
GCC_VERSION='12'
CLANG_VERSION='17'
PYTHON_VERSION='3.12'
EIGEN_VERSION='3.4.0'
RPCLIB_VERSION='2.3.0'

# Dynamic variables.
PROJECT_DIR="$(realpath $PWD)"
SCRIPT_DIR="$(realpath ${BASH_SOURCE[0]})"

if [ "$(uname)" = 'Darwin' ]; then
    SYSTEM_INFO="$(sw_vers)"
    SYSTEM_PLATFORM="$(uname -m)"  # `uname -p` only return arm/x86
else
    SYSTEM_INFO="$(system_info)"
    SYSTEM_PLATFORM="$(system_arch)"
fi
SYSTEM_CPU_MAX="$(( $(nproc) - 2 ))"
SYSTEM_OS_VERSION="$(system_os)"

###
### Main
###

#pushd "${SCRIPT_DIR}" >/dev/null  # push script directory onto the stack

# Parse command line arguments.
while [ $# -gt 0 ]; do
    key="$1"
    case $key in
    '--debug')
        DEBUG='true'
        shift
        ;;
    '--gcc')
        USE_GCC='true'
        shift
        ;;
    esac
done

# Check for existing rpclib installation.
if [ ! -d "./external/rpclib/rpclib-${RPCLIB_VERSION}" ]; then
    echo "ERROR: rpclib version not found: ${RPCLIB_VERSION}"
    echo 'Please run `setup.sh` and then `build.sh` again.'
    exit 1
fi

# Check for local cmake build created by setup.sh.
if [ -d './cmake_build' ]; then
    if [ "$(uname)" = 'Darwin' ]; then
        CMAKE="$(greadlink -f cmake_build/bin/cmake)"
    else
        CMAKE="$(readlink -f cmake_build/bin/cmake)"
    fi
else
    CMAKE="$(which cmake)"
fi
CMAKE_C_COMPILER="$CMAKE"
CMAKE_CXX_COMPILER="$CMAKE"

# Variable for build output.
if [ "${DEBUG}" = 'true' ]; then
    build_dir='build_debug'
else
    build_dir='build_release'
fi

# Configure compiler.
if [ "$(uname)" = 'Darwin' ]; then
    export CC="$(brew --prefix)/opt/llvm@${CLANG_VERSION}/bin/clang"
    export CXX="$(brew --prefix)/opt/llvm@${CLANG_VERSION}/bin/clang++"
elif [ "$(uname)" = 'Linux' ]; then
    if [ "${USE_GCC}" = 'true' ]; then
        export CC="gcc-${GCC_VERSION}"
        export CXX="g++-${GCC_VERSION}"
    else
        export CC="clang-${CLANG_VERSION}"
        export CXX="clang++-${CLANG_VERSION}"
    fi
else
    echo 'ERROR: This build script only supports Linux and MacOS.'
    exit 1
fi

# Install Eigen C++ library.
if [ ! -d "./AutonomyLib/deps/eigen3/Eigen" ]; then
    echo 'ERROR: Eigen is not installed. Please run `setup.sh` first.'
    exit 1
fi

# Ensure CMake files will be built in our build directory.
[ -f "./cmake/CMakeCache.txt" ] && rm "./cmake/CMakeCache.txt"
[ -d "./cmake/CMakeFiles" ] && rm -rf "./cmake/CMakeFiles"
[ ! -d "./cmake/${build_dir}" ] && mkdir -p "./cmake/${build_dir}"

# Enter build directory.
echo "Moving into build directory: ./cmake/${build_dir}"
pushd "./cmake/${build_dir}"

# Set CMake variables.
CMAKE_VARS=''
if [ "$(uname)" = 'Darwin' ]; then
    CMAKE_VARS="-DCMAKE_APPLE_SILICON_PROCESSOR=${SYSTEM_PLATFORM}"
fi

if [ "${DEBUG}" = 'true' ]; then
    folder_name='Debug'
    "$CMAKE" -DCMAKE_BUILD_TYPE=Debug "$CMAKE_VARS" .. || (popd && rm -rf "./$build_dir" && exit 1)
else
    folder_name='Release'
    "$CMAKE" -DCMAKE_BUILD_TYPE=Release "$CMAKE_VARS" .. || (popd && rm -rf "./$build_dir" && exit 1)
fi

# Final linking of the binaries can fail due to a missing libc++abi library
# (happens on Fedora, see https://bugzilla.redhat.com/show_bug.cgi?id=1332306).
# So we only build the libraries here for now.
make -j"${SYSTEM_CPU_MAX}"

# Drop back down into project directory.
popd

mkdir -p "./AutonomyLib/lib/x64/${folder_name}"
mkdir -p './AutonomyLib/deps/rpclib/lib'
mkdir -p './AutonomyLib/deps/MavLinkCom/lib'

cp "./cmake/${build_dir}/output/lib/libAutonomyLib.a" ./AutonomyLib/lib
cp "./cmake/${build_dir}/output/lib/libMavLinkCom.a" ./AutonomyLib/deps/MavLinkCom/lib
cp "./cmake/${build_dir}/output/lib/librpc.a" ./AutonomyLib/deps/rpclib/lib/librpc.a

# Update AutonomyLib/lib, AutonomyLib/deps, Plugins folders with new binaries
rsync -a --delete "./cmake/${build_dir}/output/lib/" "./AutonomyLib/lib/x64/${folder_name}"
rsync -a --delete "./external/rpclib/rpclib-${RPCLIB_VERSION}/include" ./AutonomyLib/deps/rpclib
rsync -a --delete ./MavLinkCom/include ./AutonomyLib/deps/MavLinkCom
rsync -a --delete ./AutonomyLib ./UnrealPlugin/Unreal/Plugins/AutonomySim/Source

rm -rf ./UnrealPlugin/Unreal/Plugins/AutonomySim/Source/AutonomyLib/src

# Update all environment projects
for d in ./UnrealPlugin/Unreal/Environments/*; do
    [ -L "${d%/}" ] && continue
    bash "${d}/scripts/clean.sh"
    mkdir -p "${d}/Plugins"
    rsync -a --delete ./UnrealPlugin/Unreal/Plugins/AutonomySim "${d}/Plugins"
done

set +x

echo '-----------------------------------------------------------------------------------------'
echo ' AutonomySim plugin built successfully.'
echo '-----------------------------------------------------------------------------------------'
echo ' All environments under Unreal/Environments have been updated.'
echo ' For further info see:'
echo '   https://github.com/nervosys/AutonomySim/blob/master/docs/build_linux.md'
echo '-----------------------------------------------------------------------------------------'

exit 0
