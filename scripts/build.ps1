<#
FILENAME:
  build.ps1
DESCRIPTION:
  PowerShell build script: AutonomySim plugin for Unreal Engine.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-22
PARAMETERS:
  - BuildMode:      [ Debug | Release | RelWithDebInfo ]
  - BuildDocs:      Enable to build and serve AutonomySim documentation.
  - CmakeGenerator: [ 'Visual Studio 17 2022' | 'Visual Studio 16 2019' ]
  - SystemDebug:    Enable for computer system debugging messages.
  - UnrealAsset:    Enable for an Unreal Engine full-polycount SUV asset.
  - Automate:       Enable to automate Visual Studio installation selection.
NOTES:
  - Assumes: PowerShell version >= 7, Unreal Engine >= 5, CMake >= 3.14, Visual Studio 2022.
  - Script is intended to run from the 'AutonomySim' base project directory.
WARNINGS:
  - PowerShell variables are case-insensitive.
  - PowerShell module names are case-insensitive (with exceptions).

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
  $BuildDocs,
  [Parameter(HelpMessage = 'Options: [ "Visual Studio 17 2022" | "Visual Studio 16 2019" ]')]
  [String]
  $CmakeGenerator = 'Visual Studio 17 2022',
  [Parameter(HelpMessage = 'Enable for computer system debugging messages.')]
  [Switch]
  $SystemDebug,
  [Parameter(HelpMessage = 'Enable for an Unreal Engine full-polycount SUV asset.')]
  [Switch]
  $UnrealAsset,
  [Parameter(HelpMessage = 'Enable for CI/CD mode (e.g., GitHub Actions).')]
  [Switch]
  $Automate
)

###
### Imports
###

# NOTE: Prefer Import-Module to Get-Content for its scoping rules.

# Utilities
# imports: Add-Directories, Remove-Directories, Invoke-Fail, Test-WorkingDirectory,
#   Test-VariableDefined, Get-EnvVariables, Get-ProgramVersion, Get-VersionMajorMinor,
#   Get-VersionMajorMinorBuild, Get-WindowsInfo, Get-WindowsVersion, Get-Architecture,
#   Get-ArchitectureWidth, Set-ProcessorCount
Import-Module "${PWD}\scripts\mod_utils.psm1"

# Documentation
Import-Module "${PWD}\scripts\mod_docs.psm1"          # imports: Build-Documentation

# Tests
Import-Module "${PWD}\scripts\mod_cmake.psm1"          # imports: CMAKE_VERSION_MINIMUM, Install-Cmake, Test-CmakeVersion
Import-Module "${PWD}\scripts\mod_eigen.psm1"          # imports: EIGEN_VERSION, Install-Eigen, Test-EigenVersion
Import-Module "${PWD}\scripts\mod_rpclib.psm1"         # imports: RPCLIB_VERSION, Install-RpcLib, Test-RpcLibVersion
Import-Module "${PWD}\scripts\mod_unrealasset.psm1"    # imports: UNREAL_ASSET_VERSION, Install-UnrealAsset, Test-UnrealAssetVersion
Import-Module "${PWD}\scripts\mod_visualstudio.psm1"   # imports: VS_VERSION_MINIMUM, Set-VsInstance, Get-VsInstanceVersion, Test-VsInstanceVersion

###
### Variables
###

# Static variables
$PROJECT_DIR = "$PWD"
$SCRIPT_DIR = "${PROJECT_DIR}\scripts"

# Command-line arguments
$AUTOMATE_MODE = if ( $Automate.IsPresent ) { $true } else { $false }
$BUILD_MODE = "$BuildMode"
$CMAKE_GENERATOR = "$CmakeGenerator"
$DEBUG_MODE = if ( $SystemDebug.IsPresent ) { $true } else { $false }
$BUILD_DOCS = if ( $BuildDocs.IsPresent ) { $true } else { $false }
$UNREAL_ASSET = if ( $UnrealAsset.IsPresent ) { $true } else { $false }

# Dynamic variables
$SYSTEM_INFO = Get-ComputerInfo  # WARNING: Windows only
$SYSTEM_PROCESSOR = "${env:PROCESSOR_IDENTIFIER}"
$SYSTEM_ARCHITECTURE = "${env:PROCESSOR_ARCHITECTURE}"
$SYSTEM_PLATFORM = Get-Architecture -Info $SYSTEM_INFO
$SYSTEM_CPU_MAX = Set-ProcessorCount -Info $SYSTEM_INFO
$SYSTEM_OS_VERSION = Get-WindowsVersion -Info $SYSTEM_INFO
$VS_INSTANCE = Set-VsInstance -Automate $Automate
$VS_VERSION = Get-VsInstanceVersion -Config $VS_INSTANCE
$CMAKE_VERSION = Get-ProgramVersion -Program 'cmake'

###
### Functions
###

# Compile AutonomySim.sln, which will also compile MavLinkCom.
function Build-Solution {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $BuildMode = 'Release',
    [Parameter(Mandatory)]
    [String]
    $SystemPlatform,
    [Parameter(Mandatory)]
    [UInt16]
    $SystemCpuMax,
    [Parameter()]
    [String[]]
    $IgnoreErrors = @()
  )
  [String]$IgnoreErrorCodes = if ( $IgnoreErrors.Count -gt 0 ) { ($IgnoreErrors -Join ';') } else { '' }
  if ( $BuildMode -eq 'Release' ) {
    Start-Process -FilePath 'msbuild.exe' -ArgumentList "-maxcpucount:${SystemCpuMax}", "-noerr:${IgnoreErrorCodes}",
      "/p:Platform=${SystemPlatform}", "/p:Configuration=Debug", 'AutonomySim.sln' -Wait -NoNewWindow -ErrorAction Stop
    Start-Process -FilePath 'msbuild.exe' -ArgumentList "-maxcpucount:${SystemCpuMax}", "-noerr:${IgnoreErrorCodes}",
      "/p:Platform=${SystemPlatform}", "/p:Configuration=Release", 'AutonomySim.sln' -Wait -NoNewWindow -ErrorAction Stop
  } else {
    Start-Process -FilePath 'msbuild.exe' -ArgumentList "-maxcpucount:${SystemCpuMax}", "-noerr:${IgnoreErrorCodes}",
      "/p:Platform=${SystemPlatform}", "/p:Configuration=${BuildMode}", 'AutonomySim.sln' -Wait -NoNewWindow -ErrorAction Stop
  }
  if ( ! $? ) { exit $LastExitCode }  # exit on error
  return $null
}

function Copy-GeneratedBinaries {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $MavLinkTargetLib = '.\AutonomyLib\deps\MavLinkCom\lib',
    [Parameter()]
    [String]
    $MavLinkTargetInclude = '.\AutonomyLib\deps\MavLinkCom\include',
    [Parameter()]
    [String]
    $AutonomyLibPluginDir = '.\Unreal\Plugins\AutonomySim\Source\AutonomyLib'
  )
  # Copy binaries and includes for MavLinkCom in deps
  New-Item -ItemType Directory -Path ("$MavLinkTargetLib", "$MavLinkTargetInclude") -Force | Out-Null
  Copy-Item -Path '.\MavLinkCom\lib' -Destination "$MavLinkTargetLib" -Recurse -Verbose
  Copy-Item -Path '.\MavLinkCom\include' -Destination "$MavLinkTargetInclude" -Recurse -Verbose
  # Copy outputs into Unreal/Plugins directory
  New-Item -ItemType Directory -Path "$AutonomyLibPluginDir" -Force | Out-Null
  Copy-Item -Path '.\AutonomyLib' -Destination "$AutonomyLibPluginDir" -Recurse -Verbose
  Copy-Item -Path '.\AutonomySim.props' -Destination "$AutonomyLibPluginDir" -Recurse -Verbose
  return $null
}

function Get-VsUnrealProjectFiles {
  [OutputType()]
  param(
    [Parameter(Mandatory)]
    [String]
    $UnrealEnvDir,
    [Parameter()]
    [String]
    $ProjectDir = "$PWD"
  )
  Set-Location "$UnrealEnvDir"
  # imports: Test-DirectoryPath, Copy-UnrealEnvItems, Remove-UnrealEnvItems, Invoke-VsUnrealProjectFileGenerator
  Import-Module "${UnrealEnvDir}\scripts\update_unreal_env.psm1"
  #Test-DirectoryPath -Path $ProjectDir
  Copy-UnrealEnvItems -Path "$ProjectDir"
  Remove-UnrealEnvItems
  Invoke-VsUnrealProjectFileGenerator
  Remove-Module "${UnrealEnvDir}\scripts\update_unreal_env.psm1"
  Set-Location "$ProjectDir"
  return $null
}

function Update-VsUnrealProjectFiles {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $ProjectDir = "$PWD",
    [Parameter()]
    [String]
    $UnrealEnvDir = '.\Unreal\Environments'
  )
  $UnrealEnvDirs = (Get-ChildItem -Path "$UnrealEnvDir" -Directory | Select-Object FullName).FullName
  foreach ( $d in $UnrealEnvDirs ) {
    Get-VsUnrealProjectFiles -UnrealEnvDir "$d" -ProjectDir "$ProjectDir"
  }
  return $null
}

###
### Main
###

if ( $DEBUG_MODE -eq $true ) {
  Write-Output (Get-WindowsInfo -Info $SYSTEM_INFO)
}

Write-Output ''
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output ' Parameters'
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output " Project directory:       $PROJECT_DIR"
Write-Output " Script directory:        $SCRIPT_DIR"
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output " Processor:               $SYSTEM_PROCESSOR"
Write-Output " Architecture:            $SYSTEM_ARCHITECTURE"
Write-Output " Platform:                $SYSTEM_PLATFORM"
Write-Output " CPU count max:           $SYSTEM_CPU_MAX"
Write-Output " Build mode:              $BUILD_MODE"
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output " Debug mode:              $DEBUG_MODE"
Write-Output " CI/CD mode:              $AUTOMATE_MODE"
Write-Output " Build docs:              $BUILD_DOCS"
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output " Windows version:         $SYSTEM_OS_VERSION"
Write-Output " Visual Studio version:   $VS_VERSION"
Write-Output " CMake version:           $CMAKE_VERSION"
Write-Output " RPClib version:          $RPCLIB_VERSION"
Write-Output " Eigen version:           $EIGEN_VERSION"
Write-Output " Unreal Asset:            $UNREAL_ASSET"
Write-Output " Unreal Asset version:    $UNREAL_ASSET_VERSION"
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output ''

# Ensure script is run from `AutonomySim` project directory.
Test-WorkingDirectory

# Test Visual Studio version (optionally automated for CI/CD).
Test-VsInstanceVersion -Automate $AUTOMATE_MODE

# Test CMake version (downloads and installs CMake).
Test-CmakeVersion

# Create temporary directories if they do not exist.
Add-Directories -Directories @('temp', 'external', 'external\rpclib')

# Test Eigen library version.
Test-EigenVersion

# Test RpcLib version (downloads and builds rpclib).
Test-RpcLibVersion -CmakeGenerator "$CMAKE_GENERATOR"

# Test high-polycount SUV asset.
Test-UnrealAssetVersion -FullPolySuv $UNREAL_ASSET

# Compile AutonomySim.sln including MavLinkCom.
Build-Solution -BuildMode "$BUILD_MODE" -SystemPlatform "$SYSTEM_PLATFORM" -SystemCpuMax "$SYSTEM_CPU_MAX"

# Copy binaries and includes for MavLinkCom and Unreal/Plugins.
Copy-GeneratedBinaries

# Update all Unreal Engine environments under AutonomySim\Unreal\Environments.
Update-VsUnrealProjectFiles -ProjectDir "$PROJECT_DIR"

# Build documentation (optional).
if ( $BUILD_DOCS ) { Build-Documentation }

exit 0
