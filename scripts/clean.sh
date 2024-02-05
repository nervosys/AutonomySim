#!/bin/bash

SCRIPT_DIR="$(realpath ${BASH_SOURCE[0]})"
pushd "$SCRIPT_DIR" > /dev/null

set -e
set +x

rm -rf build_debug
rm -rf build_release
rm -rf cmake/output

popd > /dev/null
