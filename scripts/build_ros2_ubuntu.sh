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
# - Ubuntu/ROS2 support: 20.04/foxy, 22.04/humble
# - This script is meant to be run for CI, otherwise it can break existing setups.
# TODO
#----------------------------------------------------------------------------------------

set -x  # print shell commands before executing (for debugging)
set -e  # exit on error return code

###
### Variables
###

# DISTRO="$(lsb_release -sc)"
ARCH="$(dpkg --print-architecture)"
CLANG_VERSION='12'
source /etc/os-release  # UBUNTU_CODENAME

###
### Main
###

if [ "$UBUNTU_CODENAME" = 'focal' ]; then     # Ubuntu 20.04 LTS
    ROS_DISTRO='foxy'                         # ROS2 LTS
elif [ "$UBUNTU_CODENAME" = 'jammy' ]; then   # Ubuntu 22.04 LTS
    ROS_DISTRO='humble'                       # ROS2 LTS
else
    echo "Error: Ubuntu distribution not supported: ${UBUNTU_CODENAME}"
    exit 1
fi

sudo apt-get update -y
sudo apt-get install -y locales
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8

sudo apt-get install -y curl gnupg lsb-release
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg

echo "deb [arch=${ARCH} signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu ${UBUNTU_CODENAME} main" \
    | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null

sudo apt-get update -y
sudo apt-get upgrade -y
sudo apt install -y ros-dev-tools

sudo apt-get install -y "ros-${ROS_DISTRO}-ros-base"
sudo apt-get install -y "ros-${ROS_DISTRO}-vision-opencv" "ros-${ROS_DISTRO}-image-transport" libyaml-cpp-dev "ros-${ROS_DISTRO}-geometry-msgs" \
    "ros-${ROS_DISTRO}-geographic-msgs"
sudo apt-get install -y "ros-${ROS_DISTRO}-tf2-sensor-msgs" "ros-${ROS_DISTRO}-tf2-geometry-msgs" "ros-${ROS_DISTRO}-mavros*"
sudo apt-get install -y python3-pip python3-yaml python3-setuptools python3-colcon-common-extensions

echo 'Initializing ROS2 environment...'
echo "source /opt/ros/${ROS_DISTRO}/setup.bash" >> "${HOME}/.bashrc"
source "/opt/ros/${ROS_DISTRO}/setup.bash"

echo '-------------------------------------------------------------------------------'
echo 'Success: AutonomySim ROS2 wrapper dependencies installed.'
echo '-------------------------------------------------------------------------------'
echo ''
echo 'Building AutonomySim ROS2 wrapper...'

pushd ./ros2
colcon build --cmake-args -DCMAKE_C_COMPILER="/usr/bin/clang-${CLANG_VERSION}" --cmake-args -DCMAKE_CXX_COMPILER="/usr/bin/clang++-${CLANG_VERSION}"
popd

echo ''
echo 'Success: AutonomySim ROS2 wrapper built.'

exit 0
