<#
FILENAME:
  clean.ps1
DESCRIPTION:
  PowerShell script to clean-up after build.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-20
NOTES:
  Assumes: PowerShell version >= 7, Unreal Engine >= 5, CMake >= 3.14, Visual Studio 2022.
  Script is intended to run from AutonomySim base project directory.

  Copyright © 2024 Nervosys, LLC
#>

[String]$PROJECT_DIR = (Split-Path -Parent -Path (Split-Path -Parent -Path "$PSScriptRoot"))
[String]$SCRIPT_DIR = (Split-Path -Parent -Path "$PSScriptRoot")

###
### Imports
###

# Utilities
# imports: Add-Directories, Remove-ItemSilent, Remove-TempDirectories, Invoke-Fail,
#   Test-WorkingDirectory, Test-VariableDefined, Get-EnvVariables, Get-ProgramVersion,
#   Get-VersionMajorMinor, Get-VersionMajorMinorBuild, Get-WindowsInfo, Get-WindowsVersion,
#   Get-Architecture, Get-ArchitectureWidth, Set-ProcessorCount
Import-Module "${SCRIPT_DIR}\mod_utils.psm1"

###
### Variables
###

[String]$PLATFORM = 'x86'
[String[]]$LIBRARY_PATHS = @(
  "${PROJECT_DIR}\AutonomyLib\lib", "${PROJECT_DIR}\AutonomyLib\deps\MavLinkCom",
  "${$PROJECT_DIR}\AutonomyLib\deps\rpclib", "${PROJECT_DIR}\external\rpclib\build"
)

###
### Functions
###

function CleanBuild {
  [OutputType()]
  param(
    [Parameter()]
    [String[]]
    $LibPaths = $LIBRARY_PATHS
  )
  if ( $Verbose.IsPresent ) {
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output ' Cleaning up build...'
    Write-Output '-----------------------------------------------------------------------------------------'
  }
  Remove-ItemSilent -Path $LibPaths -Recurse
  # Run MSBuild Clean for Debug and Release.
  if ( -not (Test-Program -Program "msbuild.exe") ) {
    Invoke-Fail -ErrorMessage 'Error: Program not found: msbuild.exe'
  }
  Start-Process -FilePath 'msbuild.exe' -ArgumentList "/p:Platform=${PLATFORM}", '/p:Configuration=Debug', '/t:Clean', "${PROJECT_DIR}\AutonomySim.sln" -Wait -NoNewWindow
  if ( ! $? ) { exit $LastExitCode }  # exit on error
  Start-Process -FilePath 'msbuild.exe' -ArgumentList "/p:Platform=${PLATFORM}", '/p:Configuration=Release', '/t:Clean', "${PROJECT_DIR}\AutonomySim.sln" -Wait -NoNewWindow
  if ( ! $? ) { exit $LastExitCode }  # exit on error
  return $null
}

###
### Main
###

CleanBuild

Write-Output 'Success: Removed AutonomyLib library dependency temporary files.'

exit 0
