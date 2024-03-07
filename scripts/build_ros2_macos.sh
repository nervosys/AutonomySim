#!/bin/bash
#----------------------------------------------------------------------------------------
# Filename
#   build_ros2.sh
# Description
#   BASH script to installs ROS2 dependencies on Ubuntu Linux.
# Authors
#   Adam Erickson (Nervosys)
# Date
#   2024-03-07
# Usage
#   `bash build_ros2.sh`
# Notes
# - ROS2 no longer provides binaries and must instead be build from source.
# - This recipe is for ROS2 Humble, but may work for other distros as well.
# TODO
#----------------------------------------------------------------------------------------

###
### Variables
###

ROS_DISTRO='humble'
CLANG_VERSION='12'
MACOS_VERSION="$(sw_vers -productVersion)"  # format: 10.x.x

###
### Main
###

echo '-------------------------------------------------------------------------------'
echo "Installing ROS distribution: ${ROS_DISTRO}"
echo '-------------------------------------------------------------------------------'

# Print macOS version information.
echo "$(sw_vers)"

# Print macOS system integrity protection status.
echo 'Ensuring macOS System Integrity Protection is not enabled...'
echo "$(csrutil status)" && sudo csrutil disable

# Change the default shell to BASH.
echo 'Changing the default shell to BASH...'
sudo chsh -s /bin/bash

# Ensure Homebrew is installed.
if ! command -v brew >/dev/null; then
    echo 'Installing Homebrew...'
    NONINTERACTIVE=1 /bin/bash -c "$(curl -fsSL 'https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh')"
fi

# Ensure XCode Command Line Tools are installed.
echo 'Installing XCode Command Line Tools...'
sudo rm -rf /Library/Developer/CommandLineTools
sudo xcode-select --install  # install xcode command-line tools
sudo xcodebuild -license  # accept the XCode license
echo "XCode Command Line Tools path: $(sudo xcode-select -p)"
# sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer

# Install Homebrew dependencies.
echo 'Installing Homebrew dependencies...'
brew install asio assimp bison bullet cmake console_bridge cppcheck \
    cunit eigen freetype graphviz opencv openssl orocos-kdl pcre poco \
    pyqt5 python qt@5 sip spdlog osrf/simulation/tinyxml1 tinyxml2

# Install a specific clang version.
echo "Installing LLVM/Clang version: ${CLANG_VERSION}"
brew tap homebrew/versions
brew install "llvm@${CLANG_VERSION}"

# Add OPENSSL_ROOT_DIR for DDS-Security.
echo "export OPENSSL_ROOT_DIR=$(brew --prefix openssl)" | tee -a "${HOME}/.bashrc"

# Add Qt5 to CMAKE_PREFIX_PATH and PATH.
export CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}:$(brew --prefix qt@5)"
export PATH="${PATH}:$(brew --prefix qt@5)/bin"

# Add binary install location to PATH.
export PATH="${PATH}:$(brew --prefix)/bin"

# Install ROS2 dependencies via pip.
echo 'Installing Python dependencies...'
python3 -m pip install -U \
    argcomplete catkin_pkg colcon-common-extensions coverage \
    cryptography empy flake8 flake8-blind-except==0.1.1 flake8-builtins \
    flake8-class-newline flake8-comprehensions flake8-deprecated \
    flake8-docstrings flake8-import-order flake8-quotes \
    importlib-metadata lark==1.1.1 lxml matplotlib mock mypy==0.931 netifaces \
    nose pep8 psutil pydocstyle pydot pygraphviz pyparsing==2.4.7 \
    pytest-mock rosdep rosdistro setuptools==59.6.0 vcstool

echo 'Fetching ROS2 source...'
mkdir -p "${HOME}/ros2_${ROS_DISTRO}/src"
pushd "${HOME}/ros2_${ROS_DISTRO}"
vcs import --input "https://raw.githubusercontent.com/ros2/ros2/${ROS_DISTRO}/ros2.repos" src

echo 'Building ROS2 from source...'
colcon build --symlink-install --packages-skip-by-dep python_qt_binding
popd

echo 'Initializing ROS2 environment...'
echo "source ${HOME}/ros2_${ROS_DISTRO}/install/setup.bash" >> "${HOME}/.bashrc"
source "${HOME}/ros2_${ROS_DISTRO}/install/setup.bash"

echo '-------------------------------------------------------------------------------'
echo 'Success: AutonomySim ROS2 wrapper dependencies installed.'
echo '-------------------------------------------------------------------------------'
echo ''
echo 'Building AutonomySim ROS2 wrapper...'

pushd ./ros2
colcon build --cmake-args -DCMAKE_C_COMPILER="/usr/local/bin/clang-${CLANG_VERSION}" --cmake-args -DCMAKE_CXX_COMPILER="/usr/local/bin/clang++-${CLANG_VERSION}"
popd

echo ''
echo 'Success: AutonomySim ROS2 wrapper built.'

exit 0
