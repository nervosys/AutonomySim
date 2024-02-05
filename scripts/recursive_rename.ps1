<#
FILENAME:
  recursive_rename.ps1
DESCRIPTION:
  PowerShell script to recursively rename files and directories.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  11-17-2023
NOTES:
  Assumes: PowerShell version >= 7, Unreal Engine >= 5, CMake >= 3.14, Visual Studio 2022.
  Script is intended to run from AutonomySim base project directory.

  Copyright © 2023 Nervosys, LLC
#>

$OLD='.bat'
$NEW='.cmd'

Get-ChildItem -File -Recurse | ForEach-Object { Rename-Item -Path $_.PSPath -NewName $_.Name.replace($OLD, $NEW)}

exit 0
