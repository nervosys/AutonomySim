<#
FILENAME:
  format_cpp.ps1
DESCRIPTION:
  PowerShell script to recursively format C/C++ files with clang-format.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-20
NOTES:
  Assumes: PowerShell version >= 7, clang-format version 17.
  Script is intended to run from AutonomySim base project directory.
USAGE:
  PS DRIVE:\AutonomySim> .\scripts\format_cpp.ps1

  Copyright © 2024 Nervosys, LLC
#>

###
### Imports
###

# Formatting
# imports: Install-ModuleIfMissing, Get-CppFiles, Format-CppRecursive, Get-PsFiles,
#   Format-PsFile, Format-PsRecursive
Import-Module "${PWD}\scripts\mod_format.psm1"

###
### Variables
###

# Specify C/C++ project directories to avoid scanning the entire repository.
$CPP_DIR_PATHS = @(
  "AutonomyLib", "AutonomyLibUnitTests", "DroneServer", "DroneShell", "Examples", "HelloCar",
  "HelloDrone", "HelloSpawnedDrones", "MavLinkCom", "ros2", "SemiGlobalMatching", "Unreal"
)

$CPP_FILES_INCLUDE = @("*.c", "*.cpp", "*.cc", "*.hpp", "*.h")
$CPP_FILES_EXCLUDE = @("json.hpp")

###
### Main
###

Format-CppRecursive -ProjectDirs $CPP_DIR_PATHS -Include $CPP_FILES_INCLUDE -Exclude $CPP_FILES_EXCLUDE

Write-Output 'Success: C/C++ files formatted.'

exit 0
