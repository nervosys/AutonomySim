#!/bin/bash

#SCRIPT_DIR="$(realpath ${BASH_SOURCE[0]})"
#pushd "$SCRIPT_DIR" > /dev/null

cp README.md docs/
sed -i 's/](docs\//](/g' docs/README.md
mkdocs build

#popd > /dev/null
