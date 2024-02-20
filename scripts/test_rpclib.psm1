<#
FILENAME:
  test_rpclib.psm1
DESCRIPTION:
  PowerShell script to validate RPClib version.
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

[String]$PROJECT_DIR = "$PWD"
#[String]$SCRIPT_DIR = "${PROJECT_DIR}\scripts"

[Version]$RPCLIB_VERSION = '2.3.0'
[String]$RCPLIB_VERSION_MAJ_MIN_BUILD = Get-VersionMajorMinorBuild -Version "$RPCLIB_VERSION"
[String]$RPCLIB_PATH = "external\rpclib\rpclib-${RCPLIB_VERSION_MAJ_MIN_BUILD}"
[String]$RPCLIB_URL = "https://github.com/rpclib/rpclib/archive/v${RCPLIB_VERSION_MAJ_MIN_BUILD}.zip"
[String]$VS_GENERATOR = 'Visual Studio 17 2022'

###
### Functions
###

function Get-RpcLib {
    Write-Output ''
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output " Downloading rpclib version ${RPCLIB_VERSION}..."
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output ''

    # Set security protocol used for web requests and download rpclib
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12  # TLS v1.2
    Invoke-WebRequest $RPCLIB_URL -OutFile 'temp\rpclib.zip'

    # Unpack and remove archive
    Expand-Archive -Path 'temp\rpclib.zip' -DestinationPath 'external\rpclib'
    Remove-Item 'temp\rpclib.zip'

    # Fail build if unable to download and/or unpack rpclib
    if ( -not (Test-Path -LiteralPath "$RPCLIB_PATH") ) {
        Write-Error 'Error: Unable to download rpclib. Stopping build.' -ErrorAction SilentlyContinue
        Invoke-Fail -ErrorMessage "Error: Failed to download and unpack RCPLib."
    }
}

function Build-RpcLib {
    Write-Output ''
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output " Building rpclib version ${RPCLIB_VERSION} with CMake version ${CMAKE_VERSION}..."
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output ''
    
    [System.IO.Directory]::CreateDirectory("${RPCLIB_PATH}\build")
    Set-Location "${RPCLIB_PATH}\build"
    Write-Output "Current directory: ${RPCLIB_PATH}\build"

    Start-Process -FilePath 'cmake.exe' -ArgumentList "-G ${VS_GENERATOR}", '..' -Wait -NoNewWindow
    if ( $BUILD_MODE -eq 'Release' ) {
        Start-Process -FilePath 'cmake.exe' -ArgumentList '--build', '.' -Wait -NoNewWindow
        Start-Process -FilePath 'cmake.exe' -ArgumentList '--build', '.', '--config Release' -Wait -NoNewWindow
    }
    else {
        Start-Process -FilePath 'cmake.exe' -ArgumentList '--build', '.', "--config ${BUILD_MODE}" -Wait -NoNewWindow
    }
    if (!$?) { exit $LASTEXITCODE }  # exit on error

    Set-Location "$PROJECT_DIR"
    Write-Output "Current directory: ${PROJECT_DIR}"

    # Copy rpclib binaries and include folder inside AutonomyLib folder
    $RPCLIB_TARGET_LIB = 'AutonomyLib\deps\rpclib\lib\x64'
    $RPCLIB_TARGET_INCLUDE = 'AutonomyLib\deps\rpclib\include'
    [System.IO.Directory]::CreateDirectory($RPCLIB_TARGET_LIB)
    [System.IO.Directory]::CreateDirectory($RPCLIB_TARGET_INCLUDE)

    # copy directories robustly
    Copy-Item -Path "${RPCLIB_PATH}\include" -Destination "${RPCLIB_TARGET_INCLUDE}"
    if ( $BUILD_MODE -eq 'Release' ) {
        Copy-Item -Path "${RPCLIB_PATH}\build\Debug" -Destination "${RPCLIB_TARGET_LIB}\Debug"
        Copy-Item -Path "${RPCLIB_PATH}\build\Release" -Destination "${RPCLIB_TARGET_LIB}\Release"
    }
    else {
        Copy-Item -Path "${RPCLIB_PATH}\build\${BUILD_MODE}" -Destination "${RPCLIB_TARGET_LIB}\$BUILD_MODE"
    }
}

function Test-RpcLibVersion {
    if ( -not (Test-Path -LiteralPath "$RPCLIB_PATH") ) {
        # Remove previous installations
        Remove-Item 'external\rpclib' -Force -Recurse
        # Download and build rpclib
        Get-RpcLib
        Build-RpcLib
        # Fail if rpclib version path not found
        if ( -not (Test-Path -LiteralPath "$RPCLIB_PATH") ) {
            Write-Error 'Error: Download and build of rpclib failed. Stopping build.' -ErrorAction SilentlyContinue
            Invoke-Fail -ErrorMessage "Error: Failed to download and build rpclib."
        }
    }
    else {
        Write-Output "Existing installation of rpclib version ${RPCLIB_VERSION} found."
    }
}

###
### Exports
###

Export-ModuleMember -Variable RPCLIB_VERSION
Export-ModuleMember -Function Test-RpcLibVersion
