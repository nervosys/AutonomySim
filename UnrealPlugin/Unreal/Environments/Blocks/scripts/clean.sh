#!/bin/bash

set -e
set -x

# Get path of current script
SCRIPT_DIR=$(realpath "${BASH_SOURCE[0]}")
ENV_DIR="${SCRIPT_DIR}/.."

echo "Script directory: ${SCRIPT_DIR}"
echo "Environment directory: ${ENV_DIR}"

pushd "${ENV_DIR}" >/dev/null

# clean temporary unreal folders
rm -rf ./Binaries
rm -rf ./Intermediate
rm -rf ./Saved
rm -rf ./Plugins/AutonomySim/Binaries
rm -rf ./Plugins/AutonomySim/Intermediate
rm -rf ./Plugins/AutonomySim/Saved
rm -f ./CMakeLists.txt
rm -f ./Makefile

popd >/dev/null
