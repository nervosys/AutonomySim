#!/bin/bash

# get path of current script: https://stackoverflow.com/a/39340259/207661
SCRIPT_DIR="$(realpath ${BASH_SOURCE[0]})"
pushd "$SCRIPT_DIR" > /dev/null

set -e
set -x

rsync -a  --exclude 'temp' --delete Plugins/AutonomySim ../../Plugins/
rsync -a  --exclude 'temp' --delete Plugins/AutonomySim/Source/AutonomyLib ../../../

popd > /dev/null

set +x
