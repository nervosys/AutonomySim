<#
FILENAME:
  build.ps1
DESCRIPTION:
  PowerShell build script: AutonomySim plugin for Unreal Engine.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  02-19-2024
PARAMETERS:
  - BuildMode:      [ Debug | Release | RelWithDebInfo ]
  - BuildDocs:      Enable to build and serve AutonomySim documentation.
  - FullPolySuv:    Enable for an Unreal Engine full-polycount SUV asset.
  - SystemDebug:    Enable for computer system debugging messages.
NOTES:
  Assumes: PowerShell version >= 7, Unreal Engine >= 5, CMake >= 3.14, Visual Studio 2022.
  Script is intended to run from the 'AutonomySim' base project directory.

  Copyright © 2024 Nervosys, LLC
#>

###
### Command-line interface (CLI) arguments
###

param(
    [Parameter(HelpMessage = 'Options: [ Debug | Release | RelWithDebInfo ]')]
    [String]
    $BuildMode = 'Release',
    [Parameter(HelpMessage = 'Enable to build and serve AutonomySim documentation.')]
    [Switch]
    $BuildDocs = $false,
    [Parameter(HelpMessage = 'Enable for an Unreal Engine full-polycount SUV asset.')]
    [Switch]
    $FullPolySuv = $false,
    [Parameter(HelpMessage = 'Enable for computer system debugging messages.')]
    [Switch]
    $SystemDebug = $false,
    [Parameter(HelpMessage = 'Enable for CI/CD mode (e.g., GitHub Actions).')]
    [Switch]
    $Deploy = $false    
)

###
### Imports
###

# NOTE: Prefer Import-Module to Get-Content for its scoping rules

# Common utilities:
#   Add-Directories, Remove-Directories, Invoke-Fail, Test-WorkingDirectory, Test-VariableDefined,
#   Get-EnvVariables, Get-ProgramVersion, Get-VersionMajorMinor, Get-VersionMajorMinorBuild,
#   Get-WindowsInfo, Get-WindowsVersion, Get-Architecture, Get-ArchitectureWidth, Set-ProcessorCount
Import-Module "${PWD}\scripts\utils.psm1"

# Documentation
Import-Module "${PWD}\scripts\build_docs.psm1"         # imports: Build-Documentation

# Tests
Import-Module "${PWD}\scripts\test_visualstudio.psm1"  # imports: VS_VERSION_MINIMUM, Set-VsInstance, Get-VsInstanceVersion, Test-VisualStudioVersion
Import-Module "${PWD}\scripts\test_cmake.psm1"         # imports: CMAKE_VERSION_MINIMUM, Test-CmakeVersion
Import-Module "${PWD}\scripts\test_rpclib.psm1"        # imports: RPCLIB_VERSION, Test-RpcLibVersion
Import-Module "${PWD}\scripts\test_eigen.psm1"         # imports: EIGEN_VERSION, Test-EigenVersion
Import-Module "${PWD}\scripts\test_unrealasset.psm1"   # imports: ASSET_SUV_VERSION, Test-AssetSuvVersion

###
### Variables
###

# Static variables
$PROJECT_DIR = "$PWD"
$SCRIPT_DIR = "$PROJECT_DIR\scripts"

# Command-line arguments
$BUILD_MODE = "$BuildMode"
$BUILD_DOCS = if ($BuildDocs) { $true } else { $false }
$FULL_POLY_SUV = if ($FullPolySuv) { $true } else { $false }
$DEBUG_MODE = if ($SystemDebug) { $true } else { $false }
$CI_CD_MODE = if ($Deploy -eq $true) { $true } else { $false }

# Dynamic variables
$SYSTEM_INFO = Get-ComputerInfo  # Windows only
$SYSTEM_PROCESSOR = "${env:PROCESSOR_IDENTIFIER}"
$SYSTEM_ARCHITECTURE = "${env:PROCESSOR_ARCHITECTURE}"
$SYSTEM_PLATFORM = Get-Architecture -Info $SYSTEM_INFO
$SYSTEM_CPU_MAX = Set-ProcessorCount -Info $SYSTEM_INFO
$SYSTEM_OS_VERSION = Get-WindowsVersion -Info $SYSTEM_INFO
$VS_INSTANCE = Set-VsInstance
$VS_VERSION = Get-VsInstanceVersion -Config $VS_INSTANCE
$CMAKE_VERSION = Get-ProgramVersion -Program 'cmake'

###
### Functions
###

function Build-Solution {
    [OutputType()]
    param(
        [Parameter(Mandatory)]
        [String]
        $BuildMode,
        [Parameter(Mandatory)]
        [String]
        $SystemPlatform,
        [Parameter(Mandatory)]
        [UInt8]
        $SystemCpuMax
    )
    if ( $BuildMode -eq 'Release' ) {
        Start-Process -FilePath 'msbuild.exe' -ArgumentList "-maxcpucount:$SystemCpuMax", "/p:Platform=$SystemPlatform", "/p:Configuration=Debug", 'AutonomySim.sln' -Wait -NoNewWindow
        Start-Process -FilePath 'msbuild.exe' -ArgumentList "-maxcpucount:$SystemCpuMax", "/p:Platform=$SystemPlatform", "/p:Configuration=Release", 'AutonomySim.sln' -Wait -NoNewWindow
    }
    else {
        Start-Process -FilePath 'msbuild.exe' -ArgumentList "-maxcpucount:$SystemCpuMax", "/p:Platform=$SystemPlatform", "/p:Configuration=$BuildMode", 'AutonomySim.sln' -Wait -NoNewWindow
    }
    if (!$?) { exit $LASTEXITCODE }  # exit on error
}

function Copy-GeneratedBinaries {
    # Copy binaries and includes for MavLinkCom in deps
    $MAVLINK_TARGET_LIB = 'AutonomyLib\deps\MavLinkCom\lib'
    $MAVLINK_TARGET_INCLUDE = 'AutonomyLib\deps\MavLinkCom\include'
    [System.IO.Directory]::CreateDirectory($MAVLINK_TARGET_LIB)
    [System.IO.Directory]::CreateDirectory($MAVLINK_TARGET_INCLUDE)
    Copy-Item -Path 'MavLinkCom\include' -Destination $MAVLINK_TARGET_INCLUDE
    Copy-Item -Path 'MavLinkCom\lib' -Destination $MAVLINK_TARGET_LIB

    # Copy outputs into Unreal/Plugins directory
    $AUTONOMYLIB_PLUGIN_DIR = 'Unreal\Plugins\AutonomySim\Source\AutonomyLib'
    [System.IO.Directory]::CreateDirectory($AUTONOMYLIB_PLUGIN_DIR)
    Copy-Item -Path 'AutonomyLib' -Destination $AUTONOMYLIB_PLUGIN_DIR
    Copy-Item -Path 'AutonomySim.props' -Destination $AUTONOMYLIB_PLUGIN_DIR
}

function Get-VsUnrealProjectFiles {
    param(
        [Parameter(Mandatory)]
        [String]
        $UnrealEnvDir,
        [Parameter()]
        [String]
        $ProjectDir = "$PWD"
    )
    Set-Location $UnrealEnvDir
    Import-Module "$UnrealEnvDir\scripts\update_unreal_env.psm1"  # imports: Test-DirectoryPath, Copy-UnrealEnvItems, Remove-UnrealEnvItems, Invoke-VsUnrealProjectFileGenerator
    #Test-DirectoryPath -Path $ProjectDir
    Copy-UnrealEnvItems -Path $ProjectDir
    Remove-UnrealEnvItems
    Invoke-VsUnrealProjectFileGenerator
    Remove-Module "$UnrealEnvDir\scripts\update_unreal_env.psm1"
    Set-Location $ProjectDir
}

function Update-VsUnrealProjectFiles {
    param(
        [Parameter()]
        [String]
        $ProjectDir = "$PWD"
    )
    $UnrealEnvDirs = (Get-ChildItem -Path 'Unreal\Environments' -Directory | Select-Object FullName).FullName  # remove attribute decorator
    foreach ($UnrealEnvDir in $UnrealEnvDirs) {
        Get-VsUnrealProjectFiles -UnrealEnvDir $UnrealEnvDir -ProjectDir $ProjectDir
    }
}

###
### Main
###

if ( $DEBUG_MODE -eq $true ) { Write-Output (Get-WindowsInfo($SYSTEM_INFO)) }

Write-Output ''
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output ' Parameters'
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output " Project directory:     $PROJECT_DIR"
Write-Output " Script directory:      $SCRIPT_DIR"
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output " Processor:             $SYSTEM_PROCESSOR"
Write-Output " Architecture:          $SYSTEM_ARCHITECTURE"
Write-Output " Platform:              $SYSTEM_PLATFORM"
Write-Output " CPU count max:         $SYSTEM_CPU_MAX"
Write-Output " Build mode:            $BUILD_MODE"
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output " Debug mode:            $DEBUG_MODE"
Write-Output " CI/CD mode:            $CI_CD_MODE"
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output " Windows version:       $SYSTEM_OS_VERSION"
Write-Output " Visual Studio version: $VS_VERSION"
Write-Output " CMake version:         $CMAKE_VERSION"
Write-Output " RPClib version:        $RPCLIB_VERSION"
Write-Output " Eigen version:         $EIGEN_VERSION"
Write-Output " SUV asset version:     $ASSET_SUV_VERSION"
Write-Output " Full-polycount SUV:    $FULL_POLY_SUV"
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output ''

# Ensure script is run from `AutonomySim` project directory
Test-WorkingDirectory

# Test Visual Studio version
Test-VisualStudioVersion -Automate $CI_CD_MODE

# Test CMake version
Test-CmakeVersion

# Create temporary build directories if they do not exist
Add-Directories -Directories @('temp', 'external', 'external\rpclib')

# Test RPClib version
Test-RpcLibVersion

# Test high-polycount SUV asset
Test-AssetSuvVersion -FullPolySuv $FULL_POLY_SUV

# Test Eigen library version
Test-EigenVersion

# Compile AutonomySim.sln including MavLinkCom
Build-Solution -BuildMode "$BUILD_MODE" -SystemPlatform "$SYSTEM_PLATFORM" -SystemCpuMax "$SYSTEM_CPU_MAX"

# Copy binaries and includes for MavLinkCom and Unreal/Plugins
Copy-GeneratedBinaries

# Update all Unreal Engine environments under AutonomySim\Unreal\Environments
Update-VsUnrealProjectFiles -ProjectDir "$PROJECT_DIR"

# Optionally build documentation
if ($BUILD_DOCS) { Build-Documentation }

exit 0
