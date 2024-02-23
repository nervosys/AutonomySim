<#
FILENAME:
  format_powershell.ps1
DESCRIPTION:
  PowerShell script to recursively format PowerShell files with PSScriptAnalyzer.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-22
NOTES:
  Assumes: PowerShell version >= 7, clang-format version 17.
  Script is intended to run from AutonomySim base project directory.
USAGE:
  PS DRIVE:\AutonomySim> .\scripts\format_ps.ps1

  Copyright © 2024 Nervosys, LLC
#>

###
### Imports
###

# Formatting
# imports: Install-ModuleIfMissing, Get-CppFiles, Format-CppRecursive,
#   Get-PsFiles, Format-PsFile, Format-PsRecursive
Import-Module "${PWD}\scripts\mod_format.psm1"

###
### Variables
###

# Specify PowerShell directories to avoid scanning the entire repository.
$PS_DIR_PATHS = @(".\scripts")

$PS_FILES_INCLUDE = @('*.ps1', '*.psm1', '*.psd1')
$PS_FILES_EXCLUDE = @()

###
### Main
###

Format-PowerShellRecursive -DirPaths $PS_DIR_PATHS -Include $PS_FILES_INCLUDE -Exclude $PS_FILES_EXCLUDE

Write-Output 'Success: PowerShell files formatted.'

exit 0
