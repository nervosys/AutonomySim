#!/bin/bash

SCRIPT_DIR="$(realpath ${BASH_SOURCE[0]})"
pushd "$SCRIPT_DIR" > /dev/null

set -x

git clean -ffdx
git pull

set -e

./setup.sh
./build.sh

exit 0