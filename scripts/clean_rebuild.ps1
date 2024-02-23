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
### Variables
###

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
  Remove-Item -Path 'external' -Recurse -Force
  # Run git-clean and git-pull
  Start-Process -FilePath 'git.exe' -ArgumentList 'clean', '-ffdx' -Wait -NoNewWindow
  Start-Process -FilePath 'git.exe' -ArgumentList 'pull' -Wait -NoNewWindow
  # Run build script
  . "${SCRIPT_DIR}\build.ps1"
  return $null
}

###
### Main
###

CleanRebuild

Write-Output 'Success: Removed temporary files and rebuilt project.'

exit 0
