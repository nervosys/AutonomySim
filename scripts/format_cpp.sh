#!/bin/bash
#
#----------------------------------------------------------------------------------------
# Filename
#   format_code.sh
# Description
#   BASH script to format all C/C++ project code.
# Authors
#   Microsoft (original)
#   Adam Erickson (Nervosys)
# Date
#   2024-02-20
# Usage
#   `bash format_code.sh`
# Notes
# - This script will recursively format all files under the working directory.
#----------------------------------------------------------------------------------------

DIR="$(pwd)"
FILES="$(find ${DIR} -type 'f' \( -name '*.cpp' -o -name '*.c' -o -name '*.h' -o -name '*.hpp' \) -printf '%p\n')"

printf "Formatting files:\n${FILES}" > 'format_cpp.log'

# NOTE: do not quote the `$FILES` variable below as `clang-format` will parse the entire
# list of newline-separated file paths as a single file and fail.
clang-format --verbose -i --style=file ${FILES}

echo 'C/C++ project file formatting complete.'
exit 0
