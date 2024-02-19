<#
FILENAME:
  test_rpclib.psm1
DESCRIPTION:
  PowerShell script to validate RPClib version.
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

[string]$RPCLIB_VERSION = '2.3.0'
[string]$RPCLIB_PATH = "external\rpclib\rpclib-$RPCLIB_VERSION"
[string]$RPCLIB_URL = "https://github.com/rpclib/rpclib/archive/v${RPCLIB_VERSION}.zip"
[string]$VS_GENERATOR = 'Visual Studio 17 2022'

###
### Functions
###

function Invoke-Fail {
    param(
        [Parameter(Mandatory)]
        [String[]]
        $ProjectDir = "$PWD"
    )
    Set-Location $ProjectDir
    Remove-Directories
    Write-Error 'Error: Build failed. Exiting Program.' -ErrorAction Stop
}

function Get-RpcLib {

    Write-Output ''
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output " Downloading rpclib version $RPCLIB_VERSION"
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output ''

    # Set security protocol used for web requests and download rpclib
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12  # TLS v1.2
    Invoke-WebRequest $RPCLIB_URL -OutFile 'temp\rpclib.zip'

    # Unpack and remove archive
    Expand-Archive -Path 'temp\rpclib.zip' -DestinationPath 'external\rpclib'
    Remove-Item 'temp\rpclib.zip'

    # Fail build if unable to download and/or unpack rpclib
    if ( -not (Test-Path -LiteralPath $RPCLIB_PATH) ) {
        Write-Error 'Error: Unable to download rpclib. Stopping build.' -ErrorAction SilentlyContinue
        Invoke-Fail
    }
}

function Build-RpcLib {

    Write-Output ''
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output " Building rpclib version $RPCLIB_VERSION with Cmake version $CMAKE_VERSION"
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output ''
    
    [System.IO.Directory]::CreateDirectory("$RPCLIB_PATH\build")
    Set-Location "$RPCLIB_PATH\build"
    Write-Output "Current directory: $RPCLIB_PATH\build"

    Start-Process -FilePath 'cmake.exe' -ArgumentList "-G $VS_GENERATOR", '..' -Wait -NoNewWindow
    if ( $BUILD_MODE -eq 'Release' ) {
        Start-Process -FilePath 'cmake.exe' -ArgumentList '--build', '.' -Wait -NoNewWindow
        Start-Process -FilePath 'cmake.exe' -ArgumentList '--build', '.', '--config Release' -Wait -NoNewWindow
    }
    else {
        Start-Process -FilePath 'cmake.exe' -ArgumentList '--build', '.', "--config $BUILD_MODE" -Wait -NoNewWindow
    }
    if (!$?) { exit $LASTEXITCODE }  # exit on error

    Set-Location $PROJECT_DIR
    Write-Output "Current directory: $PROJECT_DIR"

    # Copy rpclib binaries and include folder inside AutonomyLib folder
    $RPCLIB_TARGET_LIB = 'AutonomyLib\deps\rpclib\lib\x64'
    $RPCLIB_TARGET_INCLUDE = 'AutonomyLib\deps\rpclib\include'
    [System.IO.Directory]::CreateDirectory($RPCLIB_TARGET_LIB)
    [System.IO.Directory]::CreateDirectory($RPCLIB_TARGET_INCLUDE)

    # copy directories robustly
    Copy-Item -Path "$RPCLIB_PATH\include" -Destination "$RPCLIB_TARGET_INCLUDE"
    if ( $BUILD_MODE -eq 'Release' ) {
        Copy-Item -Path "$RPCLIB_PATH\build\Debug" -Destination "$RPCLIB_TARGET_LIB\Debug"
        Copy-Item -Path "$RPCLIB_PATH\build\Release" -Destination "$RPCLIB_TARGET_LIB\Release"
    }
    else {
        Copy-Item -Path "$RPCLIB_PATH\build\$BUILD_MODE" -Destination "$RPCLIB_TARGET_LIB\$BUILD_MODE"
    }
}

function Test-RpcLibVersion {
    if ( -not (Test-Path -LiteralPath $RPCLIB_PATH) ) {
        # Remove previous installations
        Remove-Item 'external\rpclib' -Force -Recurse
        # Download and build rpclib
        Get-RpcLib
        Build-RpcLib
        # Fail if rpclib version path not found
        if ( -not (Test-Path -LiteralPath $RPCLIB_PATH) ) {
            Write-Error 'Error: Download and build of rpclib failed. Stopping build.' -ErrorAction SilentlyContinue
            Invoke-Fail
        }
    }
    else {
        Write-Output "Existing installation of rpclib version $RPCLIB_VERSION found."
    }
}

###
### Exports
###

Export-ModuleMember -Variable RPCLIB_VERSION
Export-ModuleMember -Function Test-RpcLibVersion
