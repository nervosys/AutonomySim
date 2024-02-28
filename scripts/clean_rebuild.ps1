<#
FILENAME:
  clean_rebuild.ps1
DESCRIPTION:
  PowerShell script to clean and rebuild
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-20
NOTES:
  Assumes: PowerShell version >= 7, Unreal Engine >= 5, CMake >= 3.14, Visual Studio 2022.
  Script is intended to run from AutonomySim base project directory.

  Copyright © 2024 Nervosys, LLC
#>

###
### Imports
###

# Utilities
# imports: Add-Directories, Remove-ItemSilent, Remove-TempDirectories, Invoke-Fail,
#   Test-WorkingDirectory, Test-VariableDefined, Get-EnvVariables, Get-ProgramVersion,
#   Get-VersionMajorMinor, Get-VersionMajorMinorBuild, Get-WindowsInfo, Get-WindowsVersion,
#   Get-Architecture, Get-ArchitectureWidth, Set-ProcessorCount
Import-Module "${PWD}\scripts\mod_utils.psm1"

###
### Variables
###

[String]$PROJECT_DIR = "$PWD"
[String]$SCRIPT_DIR = "${PWD}\scripts"

###
### Functions
###

function CleanRebuild {
  [OutputType()]
  param()
  if ( $Verbose.IsPresent ) {
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output ' Cleaning up build and rebuilding project...'
    Write-Output '-----------------------------------------------------------------------------------------'
  }
  # Remove external directory
  Remove-ItemSilent -Path "${PROJECT_DIR}\external" -Recurse
  # Run git-clean and git-pull
  if ( -not (Test-Program -Program "git.exe") ) {
    Invoke-Fail -ErrorMessage 'Error: Program not found: git.exe'
  }
  Start-Process -FilePath 'git.exe' -ArgumentList 'clean', '-ffdx' -Wait -NoNewWindow
  Start-Process -FilePath 'git.exe' -ArgumentList 'pull' -Wait -NoNewWindow
  # Run build script
  "${SCRIPT_DIR}\build.ps1"
  return $null
}

###
### Main
###

CleanRebuild

Write-Output 'Success: Removed temporary files and rebuilt AutonomyLib.'

exit 0
