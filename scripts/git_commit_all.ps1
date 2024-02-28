<#
FILENAME:
  git_commit_all.ps1
DESCRIPTION:
  PowerShell build script for AutonomySim plugin for Unreal Engine
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
### Command-line interface (CLI) arguments
###

param(
  [Parameter(Mandatory, HelpMessage = 'Root directory of Unreal Engine environment git repositories.')]
  [string]
  $RepoRootDir,
  [Parameter(Mandatory, HelpMessage = 'Unreal Engine environment commit message.')]
  [string]
  $CommitMessage = 'Automatic update from `AutonomySim\scripts\git_commit_all.ps1`'
)

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

# Static variables
[String]$PROJECT_DIR = "$PWD"
[String]$SCRIPT_DIR = "${PROJECT_DIR}\scripts"
[String[]]$GIT_REPO_DIRS = @(
  '1919Presentation',
  'Africa',
  'AutonomySimEnvNH',
  'AncientRome',
  'ApartmentInterior',
  'B99',
  'B99_b',
  'CityEnviron',
  'Coastline',
  'DowntownCar',
  'Forest',
  'LandscapeMountains',
  'Manhatten_Test',
  'ModularCity',
  'Plains',
  'SimpleMaze',
  'TalkingHeads',
  'TrapCamera',
  'Warehouse',
  'ZhangJiaJie'
)

###
### Functions
###

function Write-Usage {
  Write-Output "Usage: ${SCRIPT_DIR}\git_commit_all.ps1 <repo root> <commit message>"
}

function Update-GitRepositories {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $GitRepoDirs = $GIT_REPO_DIRS,
    [Parameter()]
    [String]
    $ProjectDir = $PROJECT_DIR
  )
  if ( -not (Test-Program -Program 'git.exe') ) {
    Invoke-Fail -ErrorMessage 'Error: Program not found: git.exe'
  }
  Set-Location "$RepoRootDir"
  foreach ( $d in $GitRepoDirs ) {
    Write-Output "Updating git repository directory: $d"
    Set-Location "$d"
    Start-Process -FilePath 'git.exe' -ArgumentList 'add', '-A' -Wait -NoNewWindow
    Start-Process -FilePath 'git.exe' -ArgumentList 'commit', "-m ${CommitMessage}" -Wait -NoNewWindow
    Start-Process -FilePath 'git.exe' -ArgumentList 'push' -Wait -NoNewWindow
    # Exit on error; $LastExitCode = $true || $false
    if ( ! $? ) {
      Write-Error "Usage: ${SCRIPT_DIR}\git_commit_all.ps1 <repo root> <commit message>" -ErrorAction Continue
      Invoke-Fail -ErrorMessage "Error: Failed to update git repositories."
    }
    Set-Location "$d"
  }
  Set-Location "$ProjectDir"
  return $null
}

###
### Main
###

Update-GitRepositories

Write-Output 'Success: Updated git repositories.'

exit 0
