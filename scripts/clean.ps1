<#
FILENAME:
  clean.ps1
DESCRIPTION:
  PowerShell script to clean-up after build.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  11-17-2023
NOTES:
  Assumes: PowerShell version >= 7, Unreal Engine >= 5, CMake >= 3.14, Visual Studio 2022.
  Script is intended to run from AutonomySim base project directory.

  Copyright © 2023 Nervosys, LLC
#>

###
### Main
###

Write-Output '-----------------------------------------------------------------------------------------'
Write-Output ' Cleaning'
Write-Output '-----------------------------------------------------------------------------------------'

Remove-Item -Path 'AutonomyLib\lib' -Force -Recurse
Remove-Item -Path 'AutonomyLib\deps\MavLinkCom' -Force -Recurse
Remove-Item -Path 'AutonomyLib\deps\rpclib' -Force -Recurse
Remove-Item -Path 'external\rpclib\build' -Force -Recurse

Start-Process -FilePath 'msbuild.exe' -ArgumentList "/p:Platform=x64","/p:Configuration=Debug","/t:Clean",'AutonomySim.sln' -Wait -NoNewWindow
if (!$?) { exit $LASTEXITCODE }  # exit on error

Start-Process -FilePath 'msbuild.exe' -ArgumentList "/p:Platform=x64","/p:Configuration=Release","/t:Clean",'AutonomySim.sln' -Wait -NoNewWindow
if (!$?) { exit $LASTEXITCODE }  # exit on error
