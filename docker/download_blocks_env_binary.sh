#!/bin/bash

set -x

if ! which unzip; then
    sudo apt-get install unzip
fi

wget -c https://github.com/nervosys/AutonomySim/releases/download/v1.6.0-linux/Blocks.zip 
unzip -q Blocks.zip
rm Blocks.zip
