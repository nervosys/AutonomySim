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

###
### Imports
###

Import-Module "${PWD}\scripts\mod_docs.psm1"  # imports: Build-Documentation

###
### Main
###

Build-Documentation

exit 0
