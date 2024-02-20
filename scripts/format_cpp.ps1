<#
FILENAME:
  format_cpp.ps1
DESCRIPTION:
  PowerShell script to recursively format C/C++ files with clang-format.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-20
NOTES:
  Assumes: PowerShell version >= 7, clang-format version 17.
  Script is intended to run from AutonomySim base project directory.
USAGE:
  PS DRIVE:\AutonomySim> .\scripts\format_cpp.ps1

  Copyright © 2024 Nervosys, LLC
#>

# specify C/C++ project directories to avoid having to scan the entire repository.
$PROJECT_DIRS = @(
  "AutonomyLib", "AutonomyLibUnitTests", "DroneServer", "DroneShell", "Examples", "HelloCar",
  "HelloDrone", "HelloSpawnedDrones", "MavLinkCom", "ros2", "SemiGlobalMatching", "Unreal"
)

$FILES_INCLUDE = @("*.c", "*.cpp", "*.cc", "*.hpp", "*.h")
$FILES_EXCLUDE = @("json.hpp")

$CLANG_FORMAT_EXE = (Get-Command "clang-format.exe" -ErrorAction Stop).Source
$CLANG_FORMAT_CFG = (Resolve-Path -Path ".clang-format" -ErrorAction Stop).Path
$CLANG_FORMAT_LOG = ".\format_cpp.log"

function Format-CppRecursive() {
  param (
    [Parameter(Mandatory, HelpMessage = 'Path to directory containing C/C++ source code.')]
    [string]$DirPath,
    [Parameter(HelpMessage = 'Path to clang-format style configuration file.')]
    [string]$ConfigFile = ".\.clang-format",
    [Parameter(HelpMessage = 'File patterns to include.')]
    [string[]]$Include = @(),
    [Parameter(HelpMessage = 'File patterns to exclude.')]
    [string[]]$Exclude = @(),
    [Parameter(HelpMessage = 'Path to log file.')]
    [string]$LogFile = ".\clang_format.log"
  )

  Get-ChildItem -Path "$DirPath" -File -Recurse -Include $Include -Exclude $Exclude | ForEach-Object {
    Start-Process -FilePath "$CLANG_FORMAT_EXE" -ArgumentList "-i", "--style=file:${ConfigFile}", "--verbose", "$_" -NoNewWindow -Wait -RedirectStandardOutput "$LogFile" -ErrorAction Stop
  }
}

###
### Main
###

Write-Output "Found clang-format: ${CLANG_FORMAT_EXE}"
Write-Output "Found configuration file: ${CLANG_FORMAT_CFG}"
Write-Output "Writing output to log file: ${CLANG_FORMAT_LOG}"
Write-Output "Formatting C/C++ files..."

foreach ($d in $PROJECT_DIRS) {
  Format-CppRecursive -DirPath "$d" -ConfigFile "$CLANG_FORMAT_CFG" -Include $FILES_INCLUDE -Exclude $FILES_EXCLUDE -LogFile "$CLANG_FORMAT_LOG"
}

Write-Output "Formatting completed successfully."

Exit 0
