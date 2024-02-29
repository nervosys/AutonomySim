<#
FILENAME:
  build.ps1
DESCRIPTION:
  PowerShell build script: AutonomySim plugin for Unreal Engine.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-27
PARAMETERS:
  - ProjectDir:     AutonomySim base directory.
  - BuildMode:      [ Debug | Release | RelWithDebInfo ]
  - CmakeGenerator: [ Visual Studio 17 2022 | Visual Studio 16 2019 ]
  - BuildDocs:      Enable to build and serve AutonomySim documentation.
  - UnrealEnvDir:   Path to Unreal Environment directory.
  - UnrealAsset:    Enable for an Unreal Engine full-polycount SUV asset.
  - SystemDebug:    Enable for computer system debugging messages.
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
  [Parameter(HelpMessage = 'AutonomySim base directory.')]
  [String]
  $ProjectDir = "$PWD",
  [Parameter(HelpMessage = 'Options: [ Debug | Release | RelWithDebInfo ]')]
  [String]
  $BuildMode = 'Release',
  [Parameter(HelpMessage = 'Options: [ Visual Studio 17 2022 | Visual Studio 16 2019 ]')]
  [String]
  $CmakeGenerator = 'Visual Studio 17 2022',
  [Parameter(HelpMessage = 'Enable to build and serve AutonomySim documentation.')]
  [Switch]
  $BuildDocs,
  [Parameter(HelpMessage = 'Path to Unreal Environment directory.')]
  [String]
  $UnrealEnvDir,
  [Parameter(HelpMessage = 'Enable for an Unreal Engine full-polycount SUV asset.')]
  [Switch]
  $UnrealAsset,
  [Parameter(HelpMessage = 'Enable for computer system debugging messages.')]
  [Switch]
  $SystemDebug,
  [Parameter(HelpMessage = 'Enable for CI/CD mode (e.g., GitHub Actions).')]
  [Switch]
  $Automate
)

###
### Imports
###

# NOTE: Prefer Import-Module to Get-Content for its scoping rules.

# Utilities
# imports: Add-Directories, Remove-ItemSilent, Remove-TempDirectories, Invoke-Fail,
#   Test-WorkingDirectory, Test-VariableDefined, Get-EnvVariables, Get-ProgramVersion,
#   Get-VersionMajorMinor, Get-VersionMajorMinorBuild, Get-WindowsInfo, Get-WindowsVersion,
#   Get-Architecture, Get-ArchitectureWidth, Set-ProcessorCount
Import-Module "${PWD}\scripts\mod_utils.psm1"

# Build
Import-Module "${PWD}\scripts\mod_cmake.psm1"          # imports: CMAKE_VERSION_MINIMUM, Install-Cmake, Test-CmakeVersion
Import-Module "${PWD}\scripts\mod_eigen.psm1"          # imports: EIGEN_VERSION, Install-Eigen, Test-EigenVersion
Import-Module "${PWD}\scripts\mod_rpclib.psm1"         # imports: RPCLIB_VERSION, Install-RpcLib, Test-RpcLibVersion
Import-Module "${PWD}\scripts\mod_unreal.psm1"         # imports: UNREAL_ASSET_VERSION, Install-UnrealAsset, Test-UnrealAssetVersion
Import-Module "${PWD}\scripts\mod_visualstudio.psm1"   # imports: VS_VERSION_MINIMUM, Set-VsInstance, Get-VsInstanceVersion, Test-VsInstanceVersion

# Documentation
Import-Module "${PWD}\scripts\mod_docs.psm1"          # imports: Build-Documentation

###
### Variables
###

# Static variables
$PROJECT_DIR = "$ProjectDir"
$SCRIPT_DIR = "${ProjectDir}\scripts"
$UNREAL_ENV_DIR = if ( $PSBoundParameters.ContainsKey('UnrealEnvDir') ) {
  "$UnrealEnvDir"
} else {
  "${PROJECT_DIR}\UnrealPlugin\Unreal\Environments"
}

# Command-line arguments
$BUILD_MODE = "$BuildMode"
$CMAKE_GENERATOR = "$CmakeGenerator"
$DEBUG_MODE = if ( $SystemDebug.IsPresent ) { $true } else { $false }
$UNREAL_ASSET = if ( $UnrealAsset.IsPresent ) { $true } else { $false }
$BUILD_DOCS = if ( $BuildDocs.IsPresent ) { $true } else { $false }
$AUTOMATE_MODE = if ( $Automate.IsPresent ) { $true } else { $false }

# Dynamic variables
$SYSTEM_INFO = Get-ComputerInfo  # WARNING: Windows only
$SYSTEM_PROCESSOR = "${env:PROCESSOR_IDENTIFIER}"
$SYSTEM_ARCHITECTURE = "${env:PROCESSOR_ARCHITECTURE}"
$SYSTEM_PLATFORM = Get-Architecture -Info $SYSTEM_INFO
$SYSTEM_CPU_MAX = Set-ProcessorCount -Info $SYSTEM_INFO
$SYSTEM_OS_VERSION = Get-WindowsVersion -Info $SYSTEM_INFO
$VS_INSTANCE = Set-VsInstance -Automate $AUTOMATE_MODE
$VS_VERSION = Get-VsInstanceVersion -Config $VS_INSTANCE
$CMAKE_VERSION = Get-ProgramVersion -Program 'cmake'

###
### Functions
###

# Compile AutonomySim.sln, which will also compile MavLinkCom.
function Build-AutonomySim {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $ProjectDir = "$PROJECT_DIR",
    [Parameter()]
    [String]
    $BuildMode = "$BUILD_MODE",
    [Parameter()]
    [String]
    $SystemPlatform = "$SYSTEM_PLATFORM",
    [Parameter()]
    [UInt16]
    $SystemCpuMax = $SYSTEM_CPU_MAX,
    [Parameter()]
    [String[]]
    $IgnoreErrors = @()
  )
  [String]$IgnoreErrorCodes = if ( $IgnoreErrors.Count -gt 0 ) { '-noerr:' + ($IgnoreErrors -Join ';') } else { '' }
  if ( "$BuildMode" -eq 'Release' ) {
    Start-Process -FilePath 'msbuild.exe' -ArgumentList "-maxcpucount:${SystemCpuMax}", "${IgnoreErrorCodes}",
    "/p:Platform=${SystemPlatform}", '/p:Configuration=Debug', "${ProjectDir}\AutonomySim.sln" -Wait -NoNewWindow -ErrorAction Stop
    Start-Process -FilePath 'msbuild.exe' -ArgumentList "-maxcpucount:${SystemCpuMax}", "${IgnoreErrorCodes}",
    "/p:Platform=${SystemPlatform}", '/p:Configuration=Release', "${ProjectDir}\AutonomySim.sln" -Wait -NoNewWindow -ErrorAction Stop
  } else {
    Start-Process -FilePath 'msbuild.exe' -ArgumentList "-maxcpucount:${SystemCpuMax}", "${IgnoreErrorCodes}",
    "/p:Platform=${SystemPlatform}", "/p:Configuration=${BuildMode}", "${ProjectDir}\AutonomySim.sln" -Wait -NoNewWindow -ErrorAction Stop
  }
  if ( ! $? ) { exit $LastExitCode }  # exit on error
  return $null
}

function Copy-GeneratedBinaries {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $ProjectDir = "$PROJECT_DIR",
    [Parameter()]
    [String]
    $MavLinkTargetLib = "${PROJECT_DIR}\AutonomyLib\deps\MavLinkCom\lib",
    [Parameter()]
    [String]
    $MavLinkTargetInclude = "${PROJECT_DIR}\AutonomyLib\deps\MavLinkCom\include",
    [Parameter()]
    [String]
    $AutonomyLibPluginDir = "${PROJECT_DIR}\UnrealPlugin\Unreal\Plugins\AutonomySim\Source\AutonomyLib"
  )
  # Copy binaries and includes for MavLinkCom in deps
  New-Item -ItemType Directory -Path ("$MavLinkTargetLib", "$MavLinkTargetInclude") -Force | Out-Null
  Copy-Item -Path "${ProjectDir}\MavLinkCom\lib" -Destination "$MavLinkTargetLib" -Recurse -Force
  Copy-Item -Path "${ProjectDir}\MavLinkCom\include" -Destination "$MavLinkTargetInclude" -Recurse -Force
  # Copy outputs into Unreal/Plugins directory
  New-Item -ItemType Directory -Path "$AutonomyLibPluginDir" -Force | Out-Null
  Copy-Item -Path "${ProjectDir}\AutonomyLib" -Destination "$AutonomyLibPluginDir" -Recurse -Force
  Copy-Item -Path "${ProjectDir}\AutonomySim.props" -Destination "$AutonomyLibPluginDir" -Recurse -Force
  return $null
}

###
### Main
###

if ( $DEBUG_MODE -eq $true ) {
  Write-Output (Get-WindowsInfo -Info $SYSTEM_INFO)
}

if ( $Verbose.IsPresent ) {
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
}

# Ensure script is run from `AutonomySim` project directory.
Test-WorkingDirectory

# Create temporary directories if they do not exist.
Add-Directories -Directories @('temp', 'external', 'external\rpclib')

# Test Visual Studio version (optionally automated for CI/CD).
Test-VsInstanceVersion -Automate $AUTOMATE_MODE

# Test CMake version (downloads and installs CMake).
Test-CmakeVersion

# Test Eigen library version.
Test-EigenVersion

# Test RpcLib version (downloads and builds rpclib).
Test-RpcLibVersion -BuildMode "$BUILD_MODE" -CmakeGenerator "$CMAKE_GENERATOR"

# Test high-polycount SUV asset.
# Test-UnrealAssetVersion -FullPolySuv $UNREAL_ASSET

# Compile AutonomySim.sln including MavLinkCom.
Build-AutonomySim -BuildMode "$BUILD_MODE" -SystemPlatform "$SYSTEM_PLATFORM" -SystemCpuMax "$SYSTEM_CPU_MAX"

# Copy binaries and includes for MavLinkCom and Unreal/Plugins.
Copy-GeneratedBinaries

# Test Unreal SUV asset version
Test-UnrealAssetVersion -FullPolySuv $UNREAL_ASSET

# Update all Unreal Engine environments under AutonomySim\UnrealProject\Unreal\Environments.
Update-UnrealVsProjectFiles -ProjectDir "$PROJECT_DIR" -UnrealEnvRootDir "$UNREAL_ENV_DIR" -UnrealVersion $UNREAL_VERSION -Automate $AUTOMATE_MODE

# Build documentation (optional).
if ( $BUILD_DOCS ) { Build-Documentation }

exit 0
