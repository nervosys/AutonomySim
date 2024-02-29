<#
FILENAME:
  mod_rpclib.psm1
DESCRIPTION:
  PowerShell script to validate RPClib version.
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

[Version]$RPCLIB_VERSION = '2.3.0'
[String]$RCPLIB_VERSION_MAJ_MIN_BUILD = Get-VersionMajorMinorBuild -Version $RPCLIB_VERSION
[String]$RPCLIB_PATH = "${PROJECT_DIR}\external\rpclib\rpclib-${RCPLIB_VERSION_MAJ_MIN_BUILD}"
[String]$RPCLIB_URL = "https://github.com/rpclib/rpclib/archive/v${RCPLIB_VERSION_MAJ_MIN_BUILD}.zip"

[String]$BUILD_MODE = 'Release'
[String]$CMAKE_GENERATOR = 'Visual Studio 17 2022'

###
### Functions
###

function Install-RpcLib {
    [OutputType()]
    param(
        [Parameter()]
        [String]
        $RpcLibPath = "$RPCLIB_PATH",
        [Parameter()]
        [String]
        $RpcLibUrl = "$RPCLIB_URL"
    )
    if ( $Verbose.IsPresent ) {
        Write-Output '-----------------------------------------------------------------------------------------'
        Write-Output ' Installing rpclib...'
        Write-Output '-----------------------------------------------------------------------------------------'
    }
    Remove-ItemSilent -Path "${PROJECT_DIR}\temp\rpclib.zip"
    # Set security protocol used for web requests and download rpclib
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12  # TLS v1.2
    Invoke-WebRequest "$RpcLibUrl" -OutFile "${PROJECT_DIR}\temp\rpclib.zip" -HttpVersion '2.0'
    # Unpack and remove archive
    Expand-Archive -Path "${PROJECT_DIR}\temp\rpclib.zip" -DestinationPath "${PROJECT_DIR}\external\rpclib"
    Remove-ItemSilent -Path "${PROJECT_DIR}\temp\rpclib.zip"
    # Fail build if unable to download and/or unpack rpclib
    if ( -not (Test-Path -LiteralPath "$RpcLibPath") ) {
        Write-Error 'Error: Unable to download rpclib. Stopping build.' -ErrorAction SilentlyContinue
        Invoke-Fail -ErrorMessage "Error: Failed to download and unpack RCPLib."
    }
    return $null
}

function Build-RpcLib {
    [OutputType()]
    param(
        [Parameter()]
        [String]
        $BuildMode = "$BUILD_MODE",
        [Parameter()]
        [String]
        $RpcLibPath = "$RPCLIB_PATH",
        [Parameter(HelpMessage = 'Options: [ Visual Studio 17 2022 | Visual Studio 16 2019 ]')]
        [String]
        $CmakeGenerator = "$CMAKE_GENERATOR",
        [Parameter()]
        [String]
        $RpcLibTargetLib = "${PROJECT_DIR}\AutonomyLib\deps\rpclib\lib\x64",
        [Parameter()]
        [String]
        $RpcLibTargetInclude = "${PROJECT_DIR}\AutonomyLib\deps\rpclib\include"
    )
    if ( $Verbose.IsPresent ) {
        Write-Output '-----------------------------------------------------------------------------------------'
        Write-Output ' Building rpclib...'
        Write-Output '-----------------------------------------------------------------------------------------'
    }
    if ( -not (Test-Program -Program 'cmake.exe') ) {
        Invoke-Fail -ErrorMessage 'Error: cmake.exe not found.'
    }
    New-Item -ItemType Directory -Path "${RpcLibPath}\build" -Force | Out-Null
    Set-Location "${RpcLibPath}\build"
    if ( $Verbose.IsPresent ) {
        Write-Output "Current directory: ${PWD}"
        Write-Output "Using CMake generator: ${CmakeGenerator}"
    }
    # Generate RpcLib build files
    ${env:CMAKE_GENERATOR} = "$CmakeGenerator"  # [System.Environment]::GetEnvironmentVariables()
    # Generate build files
    Start-Process -FilePath 'cmake.exe' -ArgumentList @('-G', "`"$CmakeGenerator`"", '..') -Wait -NoNewWindow
    # Build RpcLib
    if ( "$BuildMode" -eq 'Release' ) {
        Start-Process -FilePath 'cmake.exe' -ArgumentList @('--build', '.') -Wait -NoNewWindow
        Start-Process -FilePath 'cmake.exe' -ArgumentList @('--build', '.', '--config', 'Release') -Wait -NoNewWindow
    } else {
        Start-Process -FilePath 'cmake.exe' -ArgumentList @('--build', '.', "--config ${BuildMode}") -Wait -NoNewWindow
    }
    if ( ! $? ) { exit $LastExitCode }  # exit on subprocess error
    Set-Location "$PROJECT_DIR"
    if ( $Verbose.IsPresent ) { Write-Output "Current directory: ${PROJECT_DIR}" }
    # Copy rpclib binaries and include folder inside AutonomyLib folder
    New-Item -ItemType Directory -Path ("$RpcLibTargetLib", "$RpcLibTargetInclude") -Force | Out-Null
    # Copy directories to targets
    Copy-Item -Path "${RpcLibPath}\include" -Destination "${RpcLibTargetInclude}" -Recurse -Force
    if ( "$BuildMode" -eq 'Release' ) {
        Copy-Item -Path "${RpcLibPath}\build\Debug" -Destination "${RpcLibTargetLib}\Debug" -Recurse -Force
        Copy-Item -Path "${RpcLibPath}\build\Release" -Destination "${RpcLibTargetLib}\Release" -Recurse -Force
    } else {
        Copy-Item -Path "${RpcLibPath}\build\${BuildMode}" -Destination "${RpcLibTargetLib}\${BuildMode}" -Recurse -Force
    }
    return $null
}

function Test-RpcLibVersion {
    [OutputType()]
    param(
        [Parameter()]
        [String]
        $BuildMode = "$BUILD_MODE",
        [Parameter()]
        [String]
        $RpcLibPath = "$RPCLIB_PATH",
        [Parameter(HelpMessage = 'Options: [ Visual Studio 17 2022 | Visual Studio 16 2019 ]')]
        [String]
        $CmakeGenerator = "$CMAKE_GENERATOR"
    )
    if ( -not (Test-Path -LiteralPath "$RpcLibPath") ) {
        # Remove previous installations
        Remove-ItemSilent -Path "${PROJECT_DIR}\external\rpclib" -Recurse
        Install-RpcLib
        Build-RpcLib -BuildMode "$BuildMode" -CmakeGenerator "$CmakeGenerator"
        # Fail if rpclib version path not found
        if ( -not (Test-Path -LiteralPath "$RpcLibPath") ) {
            Write-Error 'Error: Download and build of rpclib failed. Stopping build.' -ErrorAction SilentlyContinue
            Invoke-Fail -ErrorMessage "Error: Failed to download and build rpclib."
        }
    } else {
        Write-Output "Success: rcplib version test passed."
    }
    return $null
}

###
### Exports
###

Export-ModuleMember -Variable RPCLIB_VERSION
Export-ModuleMember -Function Install-RpcLib, Build-RpcLib, Test-RpcLibVersion
