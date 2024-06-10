<#
FILENAME:
  mod_eigen.psm1
DESCRIPTION:
  PowerShell script to validate Eigen version.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-20
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).
  
  Copyright © 2024 Nervosys, LLC
#>

[String]$PROJECT_DIR = "$PWD"
[String]$SCRIPT_DIR = "${PWD}\scripts"

###
### Imports
###

# Utilities
Import-Module "${SCRIPT_DIR}\mod_utils.psm1"

###
### Variables
###

[String]$EIGEN_VERSION = '3.4.0'
[String]$EIGEN_DIR = '.\AutonomyLib\deps\eigen3'
[String]$EIGEN_URL = "https://gitlab.com/libeigen/eigen/-/archive/${EIGEN_VERSION}/eigen-${EIGEN_VERSION}.zip"

###
### Functions
###

function Install-Eigen {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $EigenVersion = "$EIGEN_VERSION",
    [Parameter()]
    [String]
    $EigenUrl = "$EIGEN_URL",
    [Parameter()]
    [String]
    $EigenDir = "$EIGEN_DIR"
  )
  # Ensure directories exists and remove temporary files.
  Remove-ItemSilent -Path (".\temp\eigen-${EigenVersion}", '.\temp\eigen3.zip', "${EigenDir}\Eigen") -Recurse
  New-Item -ItemType Directory -Path ('.\temp', "$EigenDir") -Force | Out-Null
  if ( $Verbose.IsPresent ) {
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output ' Downloading and installing Eigen...'
    Write-Output '-----------------------------------------------------------------------------------------'
  }
  # Download and unpack library.
  [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12  # TLS v1.2
  Invoke-WebRequest "$EigenUrl" -OutFile '.\temp\eigen3.zip' -HttpVersion '2.0'
  Expand-Archive -Path '.\temp\eigen3.zip' -DestinationPath '.\temp' -Force
  # Move and delete temporary files.
  Move-Item -Path ".\temp\eigen-${EigenVersion}\Eigen" -Destination "${EigenDir}\Eigen" -Force
  # Remove temporary files.
  Remove-ItemSilent -Path (".\temp\eigen-${EigenVersion}", '.\temp\eigen3.zip') -Recurse
  return $null
}

function Test-EigenVersion {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $EigenVersion = "$EIGEN_VERSION",
    [Parameter()]
    [String]
    $EigenUrl = "$EIGEN_URL",
    [Parameter()]
    [String]
    $EigenDir = "$EIGEN_DIR"
  )
  if ( -not (Test-Path -Path "$EigenDir") ) {
    Install-Eigen -EigenVersion "$EigenVersion" -EigenUrl "$EigenUrl" -EigenDir "$EigenDir"
  }
  if ( -not (Test-Path -Path "$EigenDir") ) {
    Write-Error "Eigen library directory not found: ${EigenDir}" -ErrorAction SilentlyContinue
    Invoke-Fail -ErrorMessage "Error: Failed to download and unpack Eigen."
  }
  [String]$EigenVersFilePath = "${EigenDir}\Eigen\src\Core\util\Macros.h"
  [String]$EigenVersString = (
    (Select-String -Path "$EigenVersFilePath" -Pattern '(?<=\#define[\s]EIGEN_WORLD_VERSION[\s])[0-9]').Matches.Value,
    (Select-String -Path "$EigenVersFilePath" -Pattern '(?<=\#define[\s]EIGEN_MAJOR_VERSION[\s])[0-9]').Matches.Value,
    (Select-String -Path "$EigenVersFilePath" -Pattern '(?<=\#define[\s]EIGEN_MINOR_VERSION[\s])[0-9]').Matches.Value
  ) -Join '.'
  if ( $EigenVersString -lt $EigenVersion ) {
    Write-Output "Eigen version outdated. Installing newer version..."
    Install-Eigen -EigenVersion "$EigenVersion" -EigenUrl "$EigenUrl" -EigenDir "$EigenDir"
  }
  else {
    Write-Output "Success: Eigen version test passed."
  }
  return $null
}

###
### Exports
###

Export-ModuleMember -Variable EIGEN_VERSION
Export-ModuleMember -Function Install-Eigen, Test-EigenVersion
