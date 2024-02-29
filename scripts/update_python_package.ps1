<#
FILENAME:
  update_python_package.ps1
DESCRIPTION:
  PowerShell script to update the `autonomysim` Python package.
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
Import-Module "${SCRIPT_DIR}\mod_utils.psm1"

###
### Functions
###

function Update-PythonPackage {
    [OutputType()]
    param(
        [Parameter()]
        [Boolean]
        $PipInstall = $false
    )
    if ( -not (Test-Program -Program 'python.exe') ) {
        Invoke-Fail -ErrorMessage 'Error: Program not found: python.exe'
    }
    if ( -not (Test-Program -Program 'pip.exe') ) {
        Invoke-Fail -ErrorMessage 'Error: Program not found: pip.exe'
    }
    [String]$Confirmation = ( ($_ = Read-Host -Prompt 'Confirming package version was incremented in `setup.py` [y/N]') ? $_ : 'N' )
    if ( $Confirmation -ne 'y' ) { exit 1 }

    [String]$PYTHON_EXE = (Get-Command -Name 'python.exe').Source
    [String]$PIP_EXE = (Get-Command -Name 'pip.exe').Source
    [String]$TWINE_EXE = (Get-Command -Name 'twine.exe').Source

    Write-Output "Found the following executables:`n`tpython: $($PYTHON_EXE)`n`tpip: $($PIP_EXE)`n`ttwine: $($TWINE_EXE)"

    Start-Process -FilePath "$PYTHON_EXE" -ArgumentList @('setup.py', 'sdist') -NoNewWindow -Wait
    Start-Process -FilePath "$TWINE_EXE"  -ArgumentList @('upload', '--repository-url', 'https://upload.pypi.org/legacy/ dist/*') -NoNewWindow -Wait

    if ( $PipInstall ) {
        Start-Process -FilePath "$PIP_EXE" -ArgumentList @('install', 'autonomysim', '--upgrade') -NoNewWindow -Wait
        Start-Process -FilePath "$PIP_EXE" -ArgumentList @('show', 'autonomysim') -NoNewWindow -Wait
        Start-Process -FilePath "$PIP_EXE" -ArgumentList @('install', 'yolk3k') -NoNewWindow -Wait
        # NOTE: yolk3k is a Python tool for obtaining information about installed Python packages and querying packages avilable on PyPI.
        Start-Process -FilePath 'yolk' -ArgumentList @('-V', 'autonomysim') -NoNewWindow -Wait
    }
    return $null
}

###
### Main
###

Update-PythonPackage

Write-Output 'Success: Python package update complete.'

exit 0
