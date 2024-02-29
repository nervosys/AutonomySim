<#
FILENAME:
  recursive_rename.ps1
DESCRIPTION:
  PowerShell script to recursively rename files and directories.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-20
NOTES:
  Assumes: PowerShell version >= 7, Unreal Engine >= 5, CMake >= 3.14, Visual Studio 2022.
  Script is intended to run from AutonomySim base project directory.

  Copyright © 2024 Nervosys, LLC
#>

# Static variables
$PROJECT_DIR = "$PWD"
$SCRIPT_DIR = "${PWD}\scripts"

###
### Functions
###

function Rename-FileExtensions {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $Path = "$PROJECT_DIR",
    [Parameter()]
    [String]
    $Old = '.bat',
    [Parameter()]
    [String]
    $New = '.cmd'
  )
  Get-ChildItem -File -Recurse | ForEach-Object {
    Rename-Item -Path "$Path" -NewName $_.Name.replace("$Old", "$New")
  }
  return $null
}

###
### Main
###

Rename-FileExtensions -Old '.bat' -New '.cmd'

exit 0
