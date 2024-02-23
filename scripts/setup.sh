#!/bin/bash
#
#----------------------------------------------------------------------------------------
# Filename
#   setup.sh
# Description
#   BASH script to install and configure AutonomySim dependencies.
# Authors
#   Microsoft (original)
#   Adam Erickson (Nervosys)
# Date
#   2024-02-20
# Usage
#   `bash setup.sh --high-polycount-suv`
# Notes
# - Required: cmake, rpclib, eigen
# - Optional: high-poly count SUV asset for Unreal Engine
# - This script is a cleaned up version of the original AirSim script.
# - Assumes Unreal Engine is installed on Ubuntu 18 or MacOS 11.
# TODO
# - Update: ensure script runs on recent Linux distributions, MacOS, Windows.
#----------------------------------------------------------------------------------------

set -x  # print shell commands before executing (for debugging)
set -e  # exit on error return code

###
### Functions
###

# silence brew error if package is already installed.
function brew_install {
    brew list "$1" &>/dev/null || brew install "$1"
}

function version_less_than_equal_to {
    test "$(printf '%s\n' "$@" | sort -V | head -n 1)" = "$1"
}

###
### Variables
###

# Script directory.
SCRIPT_DIR="$(realpath ${BASH_SOURCE[0]})"

# Ensure CMake supports CMAKE_APPLE_SILICON_PROCESSOR for MacOS.
if [ "$(uname)" = 'Darwin' ]; then
    CMAKE_VERSION_MIN='3.19.2'
else
    CMAKE_VERSION_MIN='3.10.0'
fi
CMAKE_VERSION_MIN_MAJ_MIN='3.10'

CMAKE_VERSION='3.10.2'
EIGEN_VERSION='3.4.0'
RPCLIB_VERSION='2.3.0'
UNREAL_ASSET_VERSION='1.2.0'

# download high-polycount SUV model.
HIGH_POLYCOUNT_SUV='false'

DEBUG="${DEBUG:-false}"

###
### Main
###

# pushd "${SCRIPT_DIR}" >/dev/null # push script directory onto the stack

# Parse command-line interface (CLI) arguments.
while [ $# -gt 0 ]; do
    key="$1"
    case "$key" in
    '--debug')
        DEBUG='true'
        ;;
    '--high-polycount-suv')
        HIGH_POLYCOUNT_SUV='true'
        shift
        ;;
    esac
done

# Ensure LLVM and Vulkan are installed.
if [ "$(uname)" = 'Darwin' ]; then
    brew update
    brew install llvm
else
    sudo apt-get update
    sudo apt-get -y install --no-install-recommends \
        lsb-release \
        rsync \
        software-properties-common \
        wget \
        libvulkan1 \
        vulkan-utils
    VERSION=$(lsb_release -rs | cut -d. -f1)
    if [ "$VERSION" -lt '17' ]; then
        wget -O - 'http://apt.llvm.org/llvm-snapshot.gpg.key' | sudo apt-key add -
        sudo apt-get update
    fi
    sudo apt-get install -y clang-8 clang++-8 libc++-8-dev libc++abi-8-dev
fi

# Get/set CMake version.
if [ ! "$(which cmake)" ]; then
    cmake_ver='0'
else
    cmake_ver="$(cmake --version 2>&1 | head -n1 | cut -d ' ' -f3 | awk '{print $NF}')"
fi

# Give user permissions to access USB port, not needed if not using PX4 HIL.
# TODO: figure out how to do below in travis.
# Install additional tools, CMake if required.
if [ "$(uname)" = 'Darwin' ]; then
    if [ -n "${whoami}" ]; then # travis
        sudo dseditgroup -o edit -a "$(whoami)" -t user dialout
    fi
    brew update
    brew_install wget
    brew_install coreutils
    # Conditionally install lower CMake version.
    if version_less_than_equal_to "$cmake_ver" "$CMAKE_VERSION_MIN"; then
        brew install cmake
    else
        echo "Compatible version of CMake already installed: $cmake_ver"
    fi
else
    if [ -n "${whoami}" ]; then # travis
        sudo /usr/sbin/useradd -G dialout "$USER"
        sudo usermod -a -G dialout "$USER"
    fi
    sudo apt-get install -y build-essential unzip
    if version_less_than_equal_to "$cmake_ver" "$CMAKE_VERSION_MIN"; then
        if [ "$(lsb_release -rs)" = "18.04" ]; then
            sudo apt-get -y install apt-transport-https ca-certificates gnupg
            wget -O - 'https://apt.kitware.com/keys/kitware-archive-latest.asc' 2>/dev/null |
                gpg --dearmor - |
                sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
            sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
            sudo apt-get -y install --no-install-recommends make cmake
        else
            # If CMake binary not found, build it from source.
            if [ ! -d './cmake_build/bin' ]; then
                echo 'Downloading CMake...'
                wget "https://cmake.org/files/v${CMAKE_VERSION_MIN_MAJ_MIN}/cmake-${CMAKE_VERSION}.tar.gz" -O cmake.tar.gz
                tar -xzf ./cmake.tar.gz
                rm ./cmake.tar.gz
                rm -rf ./cmake_build
                mv "./cmake-${CMAKE_VERSION}" ./cmake_build
                pushd cmake_build # push directory onto stack
                ./bootstrap
                make
                popd # pop directory from stack
            fi
        fi
    else
        echo "Compatible version of CMake already installed: $cmake_ver"
    fi
fi

# Download and unpack RPCLib.
if [ ! -d "./external/rpclib/rpclib-${RPCLIB_VERSION}" ]; then
    # remove previous versions and create empty directory
    rm -rf "./external/rpclib"
    mkdir -p "./external/rpclib"
    echo '-----------------------------------------------------------------------------------------'
    echo ' Downloading RPCLib...'
    echo '-----------------------------------------------------------------------------------------'
    wget "https://github.com/rpclib/rpclib/archive/v${RPCLIB_VERSION}.zip"
    unzip -q "v${RPCLIB_VERSION}.zip" -d ./external/rpclib
    rm "v${RPCLIB_VERSION}.zip"
fi

# Download and unpack high-polycount SUV asset for Unreal Engine.
if [ "${HIGH_POLYCOUNT_SUV}" = 'true' ]; then
    if [ ! -d './Unreal/Plugins/AutonomySim/Content/VehicleAdv' ]; then
        mkdir -p './Unreal/Plugins/AutonomySim/Content/VehicleAdv'
    fi
    if [ ! -d "./Unreal/Plugins/AutonomySim/Content/VehicleAdv/SUV/v${UNREAL_ASSET_VERSION}" ]; then
        echo '-----------------------------------------------------------------------------------------'
        echo ' Downloading Unreal high-polycount SUV asset...'
        echo '-----------------------------------------------------------------------------------------'
        if [ -d ./suv_download_tmp ]; then
            rm -rf ./suv_download_tmp
        fi
        mkdir -p ./suv_download_tmp
        cd suv_download_tmp
        wget 'https://github.com/microsoft/AirSim/releases/download/v2.0.0-beta.0/car_assets.zip'
        if [ -d '../Unreal/Plugins/AutonomySim/Content/VehicleAdv/SUV' ]; then
            rm -rf ../Unreal/Plugins/AutonomySim/Content/VehicleAdv/SUV
        fi
        unzip -q ./car_assets.zip -d ../Unreal/Plugins/ASim/Content/VehicleAdv
        cd ..
        rm -rf ./suv_download_tmp
    fi
else
    echo "Skipped: Download of Unreal high-polycount SUV asset. The default Unreal Engine vehicle will be used."
fi

# Download and unpack Eigen3 C++ library.
if [ ! -d './AutonomyLib/deps/eigen3' ]; then
    echo 'Installing Eigen3 C++ library. Downloading Eigen...'
    wget -O eigen3.zip "https://gitlab.com/libeigen/eigen/-/archive/${EIGEN_VERSION}/eigen-${EIGEN_VERSION}.zip"
    unzip -q eigen3.zip -d ./temp_eigen
    mkdir -p ./AutonomyLib/deps/eigen3
    mv ./temp_eigen/eigen*/Eigen ./AutonomyLib/deps/eigen3
    rm -rf temp_eigen
    rm eigen3.zip
else
    echo "Skipped: Eigen is already installed."
fi

# popd >/dev/null # pop script directory off of the stack

set +x  # disable printing shell commands before executing

echo '-----------------------------------------------------------------------------------------'
echo ' Success: AutonomySim setup complete.'
echo '-----------------------------------------------------------------------------------------'
