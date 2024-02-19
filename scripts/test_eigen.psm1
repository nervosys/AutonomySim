<#
FILENAME:
  test_eigen.psm1
DESCRIPTION:
  PowerShell script to validate Eigen version.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  11-17-2023
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).
  Script is intended to run from AutonomySim base project directory.

  Copyright © 2024 Nervosys, LLC
#>

###
### Variables
###

[string]$EIGEN_VERSION = '3.3.7'
[string]$EIGEN_DIR = 'AutonomyLib\deps\eigen3'
[string]$EIGEN_URL = "https://gitlab.com/libeigen/eigen/-/archive/$EIGEN_VERSION/eigen-$EIGEN_VERSION.zip"

###
### Functions
###

function Remove-Directories {
  param(
      [Parameter()]
      [String[]]
      $Directories = @('temp', 'external')
  )
  foreach ($d in $Directories) {
      Remove-Item -Path "$d" -Force -Recurse
  }
}

function Invoke-Fail {
  param(
      [Parameter()]
      [String]
      $ProjectDir = "$PWD"
  )
  Set-Location $ProjectDir
  Remove-Directories
  Write-Error 'Error: Build failed. Exiting Program.' -ErrorAction Stop
}

function Test-EigenVersion {
  if ( -not (Test-Path -LiteralPath $EIGEN_DIR) ) {
    [System.IO.Directory]::CreateDirectory('AutonomyLib\deps')

    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    Invoke-WebRequest $EIGEN_URL -OutFile 'temp\eigen3.zip'
        
    Expand-Archive -Path 'temp\eigen3.zip' -DestinationPath 'AutonomyLib\deps'
    Move-Item -Path AutonomyLib\deps\eigen* -Destination 'AutonomyLib\deps\del_eigen'

    [System.IO.Directory]::CreateDirectory($EIGEN_DIR)
    Move-Item -Path 'AutonomyLib\deps\del_eigen\Eigen' -Destination "$EIGEN_DIR\Eigen"
    Remove-Item 'AutonomyLib\deps\del_eigen' -Force -Recurse
    Remove-Item 'temp\eigen3.zip' -Force
  }
  if ( -not (Test-Path -LiteralPath $EIGEN_DIR) ) {
    Write-Error "Eigen library directory not found: $EIGEN_DIR" -ErrorAction SilentlyContinue
    Invoke-Fail
  }
}

###
### Exports
###

Export-ModuleMember -Variable EIGEN_VERSION
Export-ModuleMember -Function Test-EigenVersion
