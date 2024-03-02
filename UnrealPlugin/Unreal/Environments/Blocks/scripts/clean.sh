#!/bin/bash
#----------------------------------------------------------------------------------------
# Filename
#   clean.sh
# Description
#   BASH script to cleanup Unreal Environment.
# Authors
#   Microsoft (original)
#   Adam Erickson (Nervosys)
# Date
#   2024-03-01
# Usage
# Notes
# - This script is a cleaned up version of the original AirSim script.
# TODO
#----------------------------------------------------------------------------------------

set -e
set -x

# Get path of current script
SCRIPT_DIR=$(realpath $(dirname "${BASH_SOURCE[0]}"))
ENV_DIR="${SCRIPT_DIR}/.."

echo "Script directory: ${SCRIPT_DIR}"
echo "Environment directory: ${ENV_DIR}"

pushd "${ENV_DIR}"

# clean temporary unreal folders
rm -rf ./Binaries
rm -rf ./Intermediate
rm -rf ./Saved
rm -rf ./Plugins/AutonomySim/Binaries
rm -rf ./Plugins/AutonomySim/Intermediate
rm -rf ./Plugins/AutonomySim/Saved
rm -rf ./CMakeLists.txt
rm -rf ./Makefile

popd
