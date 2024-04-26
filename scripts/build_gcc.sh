#!/bin/bash
#----------------------------------------------------------------------------------------
# Filename
#   build_gcc.sh
# Description
#   Download and build GCC from source, since Ubuntu doesn't allow you to install it.
# Authors
#   Adam Erickson (Nervosys)
# Date
#   2024-04-25
# Usage
#   `bash scripts/build_gcc.sh`
# Notes
#   This file is meant to be run from the base AutonomySim project directory.
#----------------------------------------------------------------------------------------

set -x  # print shell commands before executing (for debugging)
set -e  # exit on error return code

# Default GCC version to download and install.
# NOTE: for the latest release version see: https://mirrors.kernel.org/gnu/gcc/
GCC_VERSION='13.2.0'

# process command-line interface (CLI) arguments.
while [ $# -gt 0 ]; do
    case "$1" in
    -v|--version)
        GCC_VERSION="$2"
        echo "Downloading and building GCC version: $GCC_VERSION"
        shift  # pass argument
        shift  # pass value
        ;;
    ?)
        echo "Error: unknown argument: $1"
        exit 1
        ;;
    esac
done

# create dependencies directory if it does not exist.
mkdir -p deps
pushd deps

echo "Downloading GCC from GNU/Linux mirror: GCC ${GCC_VERSION}..."
wget "https://mirrors.kernel.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz.sig"
wget "https://mirrors.kernel.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz"

echo 'Downloading GNU public key and checking file signature...'
gpg --recv-keys 6C35B99309B5FA62
gpg --verify "gcc-${GCC_VERSION}.tar.gz.sig"

echo 'Unpacking GCC tarball...'
tar -xzvf "gcc-${GCC_VERSION}.tar.gz"

echo 'Downloading prerequisites...'
pushd "gcc-${GCC_VERSION}"
source ./contrib/download_prerequisites

echo 'Building GCC from source...'
mkdir build
pushd build
../configure --prefix="${HOME}/gcc-${GCC_VERSION}" --disable-multilib
make
make install

echo 'Removing temporary files...'
popd
popd
rm -rf "gcc-${GCC_VERSION}" "gcc-${GCC_VERSION}.tar.gz" "gcc-${GCC_VERSION}.tar.gz.sig"
popd

echo "Prepending GCC ${GCC_VERSION} binary directory path to user PATH variable..."
echo 'WARNING: This may have undesirable side-effects.'
echo "export PATH=\$HOME/gcc-${GCC_VERSION}/bin:\$PATH" | tee -a "${HOME}/.bashrc"

echo 'GCC build completed successfully.'

exit 0
