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

###
### Imports
###

# Common utilities
# imports: Add-Directories, Remove-Directories, Invoke-Fail, Test-WorkingDirectory, Test-VariableDefined,
#   Get-EnvVariables, Get-ProgramVersion, Get-VersionMajorMinor, Get-VersionMajorMinorBuild, Get-WindowsInfo,
#   Get-WindowsVersion, Get-Architecture, Get-ArchitectureWidth, Set-ProcessorCount
Import-Module "${PWD}\scripts\mod_utils.psm1"

###
### Variables
###

[String]$PROJECT_DIR = "$PWD"
[String]$SCRIPT_DIR = "${PROJECT_DIR}\scripts"

[Version]$RPCLIB_VERSION = '2.3.0'
[String]$RCPLIB_VERSION_MAJ_MIN_BUILD = Get-VersionMajorMinorBuild -Version "$RPCLIB_VERSION"
[String]$RPCLIB_PATH = "external\rpclib\rpclib-${RCPLIB_VERSION_MAJ_MIN_BUILD}"
[String]$RPCLIB_URL = "https://github.com/rpclib/rpclib/archive/v${RCPLIB_VERSION_MAJ_MIN_BUILD}.zip"

[String]$CMAKE_GENERATOR = 'Visual Studio 17 2022'

###
### Functions
###

function Install-RpcLib {
    [OutputType()]
    param(
        [Parameter()]
        [String]
        $RpcLibPath = $RPCLIB_PATH,
        [Parameter()]
        [String]
        $RpcLibUrl = $RPCLIB_URL
    )
    if ( $Verbose.IsPresent ) {
        Write-Output '-----------------------------------------------------------------------------------------'
        Write-Output ' Installing rpclib...'
        Write-Output '-----------------------------------------------------------------------------------------'
    }
    # Set security protocol used for web requests and download rpclib
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12  # TLS v1.2
    Invoke-WebRequest "$RpcLibUrl" -OutFile '.\temp\rpclib.zip' -HttpVersion '2.0'
    # Unpack and remove archive
    Expand-Archive -Path '.\temp\rpclib.zip' -DestinationPath '.\external\rpclib'
    Remove-Item '.\temp\rpclib.zip'
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
        $RpcLibPath = $RPCLIB_PATH,
        [Parameter(HelpMessage = 'Options: [ "Visual Studio 17 2022" | "Visual Studio 16 2019" ]')]
        [String]
        $CmakeGenerator = "$CMAKE_GENERATOR"
    )
    if ( $Verbose.IsPresent ) {
        Write-Output '-----------------------------------------------------------------------------------------'
        Write-Output ' Building rpclib...'
        Write-Output '-----------------------------------------------------------------------------------------'
    }
    New-Item -ItemType Directory -Path "${RpcLibPath}\build" -Force | Out-Null
    Set-Location "${RpcLibPath}\build"
    if ( $Verbose.IsPresent ) { Write-Output "Current directory: ${RpcLibPath}\build" }
    # Generate RpcLib build files
    Write-Output "Using CMake generator: ${CmakeGenerator}"
    ${env:CMAKE_GENERATOR} = "$CmakeGenerator"  # [System.Environment]::GetEnvironmentVariables()
    Start-Process -FilePath 'cmake.exe' -ArgumentList "-G `"${CmakeGenerator}"`", '..' -Wait -NoNewWindow
    # Build RpcLib
    if ( $BUILD_MODE -eq 'Release' ) {
        Start-Process -FilePath 'cmake.exe' -ArgumentList '--build', '.' -Wait -NoNewWindow
        Start-Process -FilePath 'cmake.exe' -ArgumentList '--build', '.', '--config Release' -Wait -NoNewWindow
    }
    else {
        Start-Process -FilePath 'cmake.exe' -ArgumentList '--build', '.', "--config ${BUILD_MODE}" -Wait -NoNewWindow
    }
    if ( ! $? ) { exit $LastExitCode }  # exit on subprocess error
    Set-Location "$PROJECT_DIR"
    if ( $Verbose.IsPresent ) { Write-Output "Current directory: ${PROJECT_DIR}" }
    # Copy rpclib binaries and include folder inside AutonomyLib folder
    [String]$RPCLIB_TARGET_LIB = '.\AutonomyLib\deps\rpclib\lib\x64'
    [String]$RPCLIB_TARGET_INCLUDE = '.\AutonomyLib\deps\rpclib\include'
    New-Item -ItemType Directory -Path ("$RPCLIB_TARGET_LIB", "$RPCLIB_TARGET_INCLUDE") -Force | Out-Null
    # copy directories robustly
    Copy-Item -Path "${RpcLibPath}\include" -Destination "${RPCLIB_TARGET_INCLUDE}" -Recurse -Force
    if ( $BUILD_MODE -eq 'Release' ) {
        Copy-Item -Path "${RpcLibPath}\build\Debug" -Destination "${RPCLIB_TARGET_LIB}\Debug" -Recurse -Force
        Copy-Item -Path "${RpcLibPath}\build\Release" -Destination "${RPCLIB_TARGET_LIB}\Release" -Recurse -Force
    }
    else {
        Copy-Item -Path "${RpcLibPath}\build\${BUILD_MODE}" -Destination "${RPCLIB_TARGET_LIB}\$BUILD_MODE" -Recurse -Force
    }
    return $null
}

function Test-RpcLibVersion {
    [OutputType()]
    param(
        [Parameter()]
        [String]
        $RpcLibPath = $RPCLIB_PATH,
        [Parameter(HelpMessage = 'Options: [ "Visual Studio 17 2022" | "Visual Studio 16 2019" ]')]
        [String]
        $CmakeGenerator = "$CMAKE_GENERATOR"
    )
    if ( -not (Test-Path -LiteralPath "$RpcLibPath") ) {
        # Remove previous installations
        Remove-Item '.\external\rpclib' -Force -Recurse
        Install-RpcLib
        Build-RpcLib -CmakeGenerator "$CmakeGenerator"
        # Fail if rpclib version path not found
        if ( -not (Test-Path -LiteralPath "$RpcLibPath") ) {
            Write-Error 'Error: Download and build of rpclib failed. Stopping build.' -ErrorAction SilentlyContinue
            Invoke-Fail -ErrorMessage "Error: Failed to download and build rpclib."
        }
    }
    else {
        Write-Output "Success: rcplib version test passed."
    }
    return $null
}

###
### Exports
###

Export-ModuleMember -Variable RPCLIB_VERSION
Export-ModuleMember -Function Install-RpcLib, Build-RpcLib, Test-RpcLibVersion
