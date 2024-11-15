#!/bin/bash

set -e
set -x

# get path of current script: https://stackoverflow.com/a/39340259/207661
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

pushd "$SCRIPT_DIR"

# get Unreal install directory
UnrealDir="$1"
if [ ! -d "$UnrealDir" ]; then
    UnrealDir="${SCRIPT_DIR}/UnrealEngine"
fi

# install Unreal Engine
./install_unreal.sh "$UnrealDir"

# install AutonomySim
./setup.sh
./build.sh

# start Unreal editor with Blocks project
pushd "$UnrealDir" > /dev/null
if [ "$(uname)" = 'Darwin' ]; then
    Engine/Binaries/Mac/UE4Editor.app/Contents/MacOS/UE4Editor "$SCRIPT_DIR/Unreal/Environments/Blocks/Blocks.uproject" -game -log
else
    Engine/Binaries/Linux/UE4Editor "$SCRIPT_DIR/Unreal/Environments/Blocks/Blocks.uproject" -game -log
fi

popd
popd
