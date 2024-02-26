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
### Functions
###

function CleanBuild {
  [OutputType()]
  param(
    [Parameter()]
    [String[]]
    $LibPaths = @('AutonomyLib\lib', 'AutonomyLib\deps\MavLinkCom', 'AutonomyLib\deps\rpclib', 'external\rpclib\build')
  )
  if ( $Verbose.IsPresent ) {
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output ' Cleaning up build...'
    Write-Output '-----------------------------------------------------------------------------------------'
  }
  Remove-Item -Path $LibPaths -Force -Recurse
  # Run MSBuild Clean for Debug and Release.
  Start-Process -FilePath 'msbuild.exe' -ArgumentList "/p:Platform=x64", '/p:Configuration=Debug', '/t:Clean', 'AutonomySim.sln' -Wait -NoNewWindow
  if ( ! $? ) { exit $LastExitCode }  # exit on error
  Start-Process -FilePath 'msbuild.exe' -ArgumentList "/p:Platform=x64", '/p:Configuration=Release', '/t:Clean', 'AutonomySim.sln' -Wait -NoNewWindow
  if ( ! $? ) { exit $LastExitCode }  # exit on error
  return $null
}

###
### Main
###

CleanBuild

Write-Output 'Success: Removed temporary files.'

exit 0
