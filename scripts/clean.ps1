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
[String]$PLATFORM = 'x86'
[String[]]$LIBRARY_PATHS = @("${PWD}\AutonomyLib\lib", "${PWD}\AutonomyLib\deps\MavLinkCom", "${$PWD}\AutonomyLib\deps\rpclib", "${PWD}\external\rpclib\build")

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
