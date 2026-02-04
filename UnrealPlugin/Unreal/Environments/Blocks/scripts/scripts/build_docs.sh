#!/bin/bash

echo 'Building documentation...'

cp ./README.md ./docs/
sed -i 's/](docs\//](/g' ./docs/README.md
mkdocs build

echo 'Documentation built successfully.'

exit 0
