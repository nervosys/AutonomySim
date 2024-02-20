<#
FILENAME:
  test_eigen.psm1
DESCRIPTION:
  PowerShell script to validate Eigen version.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  02-19-2024
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).
  
  Copyright © 2024 Nervosys, LLC
#>

###
### Imports
###

# Common utilities
Import-Module "${PWD}\scripts\utils.psm1"               # imports: Add-Directories, Remove-Directories, Invoke-Fail, Test-WorkingDirectory,
                                                        # Test-VariableDefined, Get-EnvVariables, Get-ProgramVersion, Get-VersionMajorMinor,
                                                        # Get-VersionMajorMinorBuild, Get-WindowsInfo, Get-WindowsVersion, Get-Architecture,
                                                        # Get-ArchitectureWidth, Set-ProcessorCount

###
### Variables
###

[String]$EIGEN_VERSION = '3.3.7'
[String]$EIGEN_DIR = 'AutonomyLib\deps\eigen3'
[String]$EIGEN_URL = "https://gitlab.com/libeigen/eigen/-/archive/${EIGEN_VERSION}/eigen-${EIGEN_VERSION}.zip"

###
### Functions
###

function Test-EigenVersion {
  if ( -not (Test-Path -LiteralPath "$EIGEN_DIR") ) {
    [System.IO.Directory]::CreateDirectory('AutonomyLib\deps')

    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    Invoke-WebRequest "$EIGEN_URL" -OutFile 'temp\eigen3.zip'
        
    Expand-Archive -Path 'temp\eigen3.zip' -DestinationPath 'AutonomyLib\deps'
    Move-Item -Path AutonomyLib\deps\eigen* -Destination 'AutonomyLib\deps\del_eigen'

    [System.IO.Directory]::CreateDirectory("$EIGEN_DIR")
    Move-Item -Path 'AutonomyLib\deps\del_eigen\Eigen' -Destination "$EIGEN_DIR\Eigen"
    Remove-Item 'AutonomyLib\deps\del_eigen' -Force -Recurse
    Remove-Item 'temp\eigen3.zip' -Force
  }
  if ( -not (Test-Path -LiteralPath "$EIGEN_DIR") ) {
    Write-Error "Eigen library directory not found: ${EIGEN_DIR}" -ErrorAction SilentlyContinue
    Invoke-Fail -ErrorMessage "Error: Failed to download and unpack Eigen."
  }
}

###
### Exports
###

Export-ModuleMember -Variable EIGEN_VERSION
Export-ModuleMember -Function Test-EigenVersion
