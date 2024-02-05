#!/bin/bash

# Get path of current script
SCRIPT_DIR="$(realpath ${BASH_SOURCE[0]})"
pushd "$SCRIPT_DIR" > /dev/null

set -e
set -x

# clean temporary unreal folders
rm -rf Binaries
rm -rf Intermediate
rm -rf Saved
rm -rf Plugins/AutonomySim/Binaries
rm -rf Plugins/AutonomySim/Intermediate
rm -rf Plugins/AutonomySim/Saved
rm -f CMakeLists.txt
rm -f Makefile

popd > /dev/null
