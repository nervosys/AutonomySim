#!/bin/bash
#
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

function system_info {
    uname -a && lscpu
}

function system_architecture {
    # unreliable: uname -m
    lscpu | grep 'Architecture' | awk {'print $2'}
}

function system_os_version {
    uname -r
}

function version_less_than_equal_to {
    test "$(printf '%s\n' "$@" | sort -V | head -n 1)" = "$1"
}

###
### Variables
###

# Script directory.
SCRIPT_DIR="$(realpath ${BASH_SOURCE[0]})"

# Static variables.
DEBUG='false'
GCC='false'

CMAKE_VERSION='3.10.2'
EIGEN_VERSION='3.4.0'
RPCLIB_VERSION='2.3.0'
UNREAL_ASSET_VERSION='1.2.0'

# Dynamic variables.
SYSTEM_INFO="$(system_info)"
SYSTEM_PLATFORM="$(system_architecture)"
SYSTEM_CPU_MAX="$(( $(nproc) - 2 ))"
SYSTEM_OS_VERSION="$(system_os_version)"

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
        GCC='true'
        shift
        ;;
    esac
done

# Check for existing rpclib installation.
if [ ! -d "./external/rpclib/rpclib-${RPCLIB_VERSION}" ]; then
    echo "ERROR: new version of AutonomySim requires newer rpclib."
    echo "please run setup.sh first and then run build.sh again."
    exit 1
fi

# Check for local cmake build created by setup.sh.
if [ -d "./cmake_build" ]; then
    if [ "$(uname)" = "Darwin" ]; then
        CMAKE="$(greadlink -f cmake_build/bin/cmake)"
    else
        CMAKE="$(readlink -f cmake_build/bin/cmake)"
    fi
else
    CMAKE="$(which cmake)"
fi

# Variable for build output.
if [ "${DEBUG}" = 'true' ]; then
    build_dir='build_debug'
else
    build_dir='build_release'
fi

# Configure compiler.
if [ "$(uname)" = 'Darwin' ]; then
    export CC="$(brew --prefix)/opt/llvm/bin/clang"
    export CXX="$(brew --prefix)/opt/llvm/bin/clang++"
elif [ "$(uname)" = 'Linux' ]; then
    if [ "${GCC}" = 'true' ]; then
        export CC="gcc-8"
        export CXX="g++-8"
    else
        export CC="clang-8"
        export CXX="clang++-8"
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

echo "Moving build into ${build_dir} directory. To clean, delete the directory."

# Ensure CMake files will be built in our build directory.
[ -f "./cmake/CMakeCache.txt" ] && rm "./cmake/CMakeCache.txt"
[ -d "./cmake/CMakeFiles" ] && rm -rf "./cmake/CMakeFiles"
[ ! -d "$build_dir" ] && mkdir -p "$build_dir"

# Fix for Unreal on Apple/ARM silicon using x86_64 (Rosetta).
CMAKE_VARS=''
[ "$(uname)" = 'Darwin' ] && CMAKE_VARS='-DCMAKE_APPLE_SILICON_PROCESSOR=x86_64'

pushd "$build_dir" >/dev/null  # push directory onto stack

if [ "${DEBUG}" = 'true' ]; then
    folder_name='Debug'
    "$CMAKE" ../cmake -DCMAKE_BUILD_TYPE=Debug $CMAKE_VARS || (popd && rm -r "$build_dir" && exit 1)
else
    folder_name='Release'
    "$CMAKE" ../cmake -DCMAKE_BUILD_TYPE=Release $CMAKE_VARS || (popd && rm -r "$build_dir" && exit 1)
fi

#popd >/dev/null                # pop directory from stack
#pushd "$build_dir" >/dev/null  # push directory onto stack

# final linking of the binaries can fail due to a missing libc++abi library
# (happens on Fedora, see https://bugzilla.redhat.com/show_bug.cgi?id=1332306).
# So we only build the libraries here for now
make -j"${SYSTEM_CPU_MAX}"
popd > /dev/null  # pop directory from stack

mkdir -p "AutonomyLib/lib/x64/${folder_name}"
mkdir -p 'AutonomyLib/deps/rpclib/lib'
mkdir -p 'AutonomyLib/deps/MavLinkCom/lib'

cp "${build_dir}/output/lib/libAutonomyLib.a AutonomyLib/lib"
cp "${build_dir}/output/lib/libMavLinkCom.a AutonomyLib/deps/MavLinkCom/lib"
cp "${build_dir}/output/lib/librpc.a AutonomyLib/deps/rpclib/lib/librpc.a"

# Update AutonomyLib/lib, AutonomyLib/deps, Plugins folders with new binaries
rsync -a --delete "${build_dir}/output/lib/ AutonomyLib/lib/x64/${folder_name}"
rsync -a --delete "external/rpclib/rpclib-${RPCLIB_VERSION}/include AutonomyLib/deps/rpclib"
rsync -a --delete 'MavLinkCom/include AutonomyLib/deps/MavLinkCom'
rsync -a --delete 'AutonomyLib Unreal/Plugins/AutonomySim/Source'

rm -rf 'Unreal/Plugins/AutonomySim/Source/AutonomyLib/src'

# Update all environment projects
for d in ./Unreal/Environments/*; do
    [ -L "${d%/}" ] && continue
    "${d}/clean.sh"
    mkdir -p "${d}/Plugins"
    rsync -a --delete Unreal/Plugins/AutonomySim "${d}/Plugins"
done

set +x

echo '-----------------------------------------------------------------------------------------'
echo ' AutonomySim plugin built successfully.'
echo '-----------------------------------------------------------------------------------------'
echo ' All environments under Unreal/Environments have been updated.'
echo ' For further info see:'
echo '   https://github.com/nervosys/AutonomySim/blob/master/docs/build_linux.md'
echo '-----------------------------------------------------------------------------------------'

popd >/dev/null  # pop directory from stack

exit 0
