<#
FILENAME:
  git_commit_all.ps1
DESCRIPTION:
  PowerShell build script for AutonomySim plugin for Unreal Engine
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  11-17-2023
NOTES:
  Assumes: PowerShell version >= 7, Unreal Engine >= 5, CMake >= 3.14, Visual Studio 2022.
  Script is intended to run from AutonomySim base project directory.

  Copyright © 2024 Nervosys, LLC
#>

# Command-line arguments
param(
  [Parameter(Mandatory, HelpMessage = 'Root directory of Unreal Engine environment git repositories.')]
  [string]
  $RepoRootDir,
  [Parameter(Mandatory, HelpMessage = 'Unreal Engine environment commit message.')]
  [string]
  $CommitMessage = 'Automatic update from `AutonomySim\scripts\git_commit_all.ps1`'
)

###
### Variables
###

# Static variables
$PROJECT_DIR = "$PWD"
$SCRIPT_DIR = "$PROJECT_DIR\scripts"
$GIT_REPO_DIRS = [String[]](
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

# Command-line arguments
$REPO_ROOT_DIR = $RepoRootDir
$COMMIT_MESSAGE = $CommitMessage

###
### Functions
###

function Invoke-Fail {
  param(
    [Parameter(Mandatory)]
    [ErrorRecord]
    $ErrorCode
  )
  Write-Output "Usage: $SCRIPT_DIR\git_commit_all.ps1 <repo root> <commit message>"
  Write-Error "Error occured while updating git repositories: $ErrorCode" -ErrorAction Stop
}

function Update-GitRepositories {
  Set-Location $REPO_ROOT_DIR
  foreach ($RepoDir in $GIT_REPO_DIRS) {
    Write-Output "Updating git repository directory: $RepoDir"
    Set-Location $RepoDir
    Start-Process -FilePath 'git.exe' -ArgumentList 'add', '-A' -Wait -NoNewWindow
    Start-Process -FilePath 'git.exe' -ArgumentList 'commit', "-m $COMMIT_MESSAGE" -Wait -NoNewWindow
    Start-Process -FilePath 'git.exe' -ArgumentList 'push' -Wait -NoNewWindow
    if (!$?) { Invoke-Fail -ErrorCode $LASTEXITCODE }  # exit on error
    Set-Location $REPO_ROOT_DIR
  }
  Set-Location $PROJECT_DIR
}

###
### Main
###

Update-GitRepositories

exit 0
