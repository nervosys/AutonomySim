<#
FILENAME:
  build_docs.ps1
DESCRIPTION:
  PowerShell script to build documentation.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-20
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).
  Script is intended to run from AutonomySim base project directory.

  Copyright © 2024 Nervosys, LLC
#>

[String]$PROJECT_DIR = "$PWD"
[String]$SCRIPT_DIR = "${PWD}\scripts"

###
### Imports
###

Import-Module "${SCRIPT_DIR}\mod_docs.psm1"  # imports: Build-Documentation

###
### Main
###

Build-Documentation

Write-Output 'Success: Building C++ API, Python API, and AutonomySim project documentation complete.'

exit 0
