#!/bin/bash
#
#----------------------------------------------------------------------------------------
# Filename
#   recursive_rename.sh
# Description
#   Script to recursively rename files and directories using `find`, `sed`, and `mv`.
# Author
#   Adam Erickson (Nervosys)
# Date
#   2023-10-18
# Notes
#   Recursively finds files and subdirectories under current directory, echoes their
#   original name `p` and then a modified name `s/${OLD}/${NEW}/g` and feeds both
#   arguments into `mv` with `xargs -n2`. Applies the `sed -z`, `find -print0`, and
#   `xargs -0` options to safely mitigate whitespace in the paths.
#----------------------------------------------------------------------------------------

set +e  # do not exit on error return code

OLD='.bat'
NEW='.cmd'

echo 'Batch renaming files...'

# Prevent ordering errors by doing the following:
# - modifying directory names first and then file names
# - starting at the working directory and walking down the directory tree
MAXDEPTH=$(find . -type d -printf '%d\n' | sort -rn | head -1)
for depth in $(seq 1 $MAXDEPTH); do
  find . -maxdepth "$depth" -type 'd' -name "*${OLD}*" -print0 | sed -ze "p;s/${OLD}/${NEW}/g" | xargs -0 -n2 mv 2>/dev/null
done

# Modify filenames recursively (at all depths)
find . -type 'f' -name "*${OLD}*" -print0 | sed -ze "p;s/${OLD}/${NEW}/g" | xargs -0 -n2 mv 2>/dev/null

echo 'Batch file renaming complete.'
exit 0
