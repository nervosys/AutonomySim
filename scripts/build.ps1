<#
FILENAME:
  build.ps1
DESCRIPTION:
  PowerShell build script for AutonomySim plugin for Unreal Engine
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  11-17-2023
NOTES:
  Assumes: PowerShell version >= 7, Unreal Engine >= 5, CMake >= 3.14, Visual Studio 2022.
  Script is intended to run from AutonomySim base project directory.

  Copyright © 2023 Nervosys, LLC
#>

# Command-line arguments
param(
    [Parameter(Mandatory=$true, HelpMessage='Options: [ Debug | Release | RelWithDebInfo ]')]
    [String]
    $BuildMode = 'Release',
    [Parameter(Mandatory=$false, HelpMessage='Enable to build and serve AutonomySim documentation')]
    [Switch]
    $BuildDocs,
    [Parameter(Mandatory=$false, HelpMessage='Enable for an Unreal Engine full-polycount SUV asset')]
    [Switch]
    $FullPolycountSuv,
    [Parameter(Mandatory=$false, HelpMessage='Enable for computer system debugging messages')]
    [Switch]
    $SystemDebug
)

###
### Imports
###

# Prefer Import-Module to Get-Content for its scoping rules
Import-Module "$SCRIPT_DIR\test_visualstudio.psm1"  # imports: VS_VERSION_MINIMUM, Set-VsInstance, Get-VsInstanceVersion, Test-VisualStudioVersion
Import-Module "$SCRIPT_DIR\test_cmake.psm1"         # imports: CMAKE_VERSION_MINIMUM, Test-CmakeVersion
Import-Module "$SCRIPT_DIR\test_rpclib.psm1"        # imports: RPCLIB_VERSION, Test-RpcLibVersion
Import-Module "$SCRIPT_DIR\test_eigen.psm1"         # imports: EIGEN_VERSION, Test-EigenVersion
Import-Module "$SCRIPT_DIR\test_unrealasset.psm1"   # imports: ASSET_SUV_VERSION, Test-AssetSuvVersion
Import-Module "$SCRIPT_DIR\build_docs.psm1"         # imports: Build-Documentation

###
### Variables
###

# Static variables
$PROJECT_DIR         = "$PWD"
$SCRIPT_DIR          = "$PROJECT_DIR\scripts"

# Command-line arguments
$BUILD_MODE          = "$BuildMode"
$BUILD_DOCS          = if ($BuildDocs) { $true } else { $false }
$FULL_POLYCOUNT_SUV  = if ($FullPolycountSuv) { $true } else { $false }
$DEBUG               = if ($SystemDebug) { $true } else { $false }

# Dynamic variables
$SYSTEM_INFO         = Get-ComputerInfo  # Windows only
$SYSTEM_PROCESSOR    = "${env:PROCESSOR_IDENTIFIER}"
$SYSTEM_ARCHITECTURE = "${env:PROCESSOR_ARCHITECTURE}"
$SYSTEM_PLATFORM     = Get-Architecture -Info $SYSTEM_INFO
$SYSTEM_CPU_MAX      = Set-ProcessorCount -Info $SYSTEM_INFO
$SYSTEM_OS_VERSION   = Get-WindowsVersion -Info $SYSTEM_INFO
$VS_INSTANCE         = Set-VsInstance
$VS_VERSION          = Get-VsInstanceVersion -Config $VS_INSTANCE
$CMAKE_VERSION       = Get-ProgramVersion -Program 'cmake'

###
### Functions
###

function List-EnvVariables {
    return Get-ChildItem 'env:*' | Sort-Object 'Name' | Format-List
}

function Test-WorkingDirectory {
    $WorkingDirectory = Split-Path $PWD -Leaf
    if ($WorkingDirectory -ne 'AutonomySim' ) {
        Write-Output "Present working directory: $PWD"
        Write-Error "Error: Script must be run from 'AutonomySim' project directory." -ErrorAction Stop
    }
}

function Invoke-Fail {
    Set-Location $PROJECT_DIR
    Remove-Directories
    Write-Error 'Error: Build failed. Exiting Program.' -ErrorAction Stop
}

function Add-Directories {
    [System.IO.Directory]::CreateDirectory('temp')
    [System.IO.Directory]::CreateDirectory('external')
    [System.IO.Directory]::CreateDirectory('external\rpclib')
}

function Remove-Directories {
    Remove-Item -Path 'temp' -Force -Recurse
    Remove-Item -Path 'external' -Force -Recurse
}

function Test-VariableDefined {
    param(
        [Parameter(Mandatory=$true)]
        [String]
        $Variable
    )
    return [Boolean](Get-Variable $Variable -ErrorAction SilentlyContinue)
}

function Get-WindowsInfo {
    param(
        [Parameter(Mandatory=$true)]
        [System.Object]
        $Info
    )
    return $Info | Select-Object WindowsProductName,WindowsVersion,OsHardwareAbstractionLayer
}

function Get-WindowsVersion {
    param(
        [Parameter(Mandatory=$true)]
        [System.Object]
        $Info
    )
    return [Version]$Info.OsHardwareAbstractionLayer
}

function Get-Architecture {
    [OutputType([String])]
    param(
        [Parameter(Mandatory=$true)]
        [System.Object]
        $Info
    )
    $arch = switch ($Info.CsSystemType) {
        'x64-based PC' {'x64'}
        'x86-based PC' {'x86'}
        $null {$null}
    }
    return $arch
}

function Get-ArchitectureWidth {
    [OutputType([String])]
    $archWidth = switch ([intptr]::Size) {
        4 {'32-bit'}
        8 {'64-bit'}
    }
    return $archWidth
}

function Set-ProcessorCount {
    [OutputType([UInt32])]
    param(
        [Parameter(Mandatory=$true)]
        [System.Object]
        $Info,
        [Parameter(HelpMessage='The number of processor cores remaining. Use all others for MSBuild.')]
        [UInt32]
        $Remainder = 2
    )
    return [UInt32]$Info.CsNumberOfLogicalProcessors - $Remainder
}

function Get-ProgramVersion {
    [OutputType([Version])]
    param(
        [Parameter(Mandatory=$true)]
        [String]
        $Program
    )
    return (Get-Command -Name $Program -ErrorAction SilentlyContinue).Version
}

function Get-VersionMajorMinor {
    [OutputType([String])]
    param(
        [Parameter(Mandatory=$true)]
        [Version]
        $Version
    )
    return $Version.Major, $Version.Minor -join '.'
}

function Get-VersionMajorMinorBuild {
    [OutputType([String])]
    param(
        [Parameter(Mandatory=$true)]
        [Version]
        $Version
    )
    return $Version.Major, $Version.Minor, $Version.Build -join '.'
}

function Build-Solution {
    if ( $BUILD_MODE -eq 'Release' ) {
        Start-Process -FilePath 'msbuild.exe' -ArgumentList "-maxcpucount:$SYSTEM_CPU_MAX","/p:Platform=$SYSTEM_PLATFORM","/p:Configuration=Debug",'AutonomySim.sln' -Wait -NoNewWindow
        Start-Process -FilePath 'msbuild.exe' -ArgumentList "-maxcpucount:$SYSTEM_CPU_MAX","/p:Platform=$SYSTEM_PLATFORM","/p:Configuration=Release",'AutonomySim.sln' -Wait -NoNewWindow
    } else {
        Start-Process -FilePath 'msbuild.exe' -ArgumentList "-maxcpucount:$SYSTEM_CPU_MAX","/p:Platform=$SYSTEM_PLATFORM","/p:Configuration=$BUILD_MODE",'AutonomySim.sln' -Wait -NoNewWindow
    }
    if (!$?) { exit $LASTEXITCODE }  # exit on error
}

function Copy-GeneratedBinaries {
    # Copy binaries and includes for MavLinkCom in deps
    $MAVLINK_TARGET_LIB     = 'AutonomyLib\deps\MavLinkCom\lib'
    $MAVLINK_TARGET_INCLUDE = 'AutonomyLib\deps\MavLinkCom\include'
    [System.IO.Directory]::CreateDirectory($MAVLINK_TARGET_LIB)
    [System.IO.Directory]::CreateDirectory($MAVLINK_TARGET_INCLUDE)
    Copy-Item -Path 'MavLinkCom\include' -Destination $MAVLINK_TARGET_INCLUDE
    Copy-Item -Path 'MavLinkCom\lib'     -Destination $MAVLINK_TARGET_LIB

    # Copy outputs into Unreal/Plugins directory
    $AUTONOMYLIB_PLUGIN_DIR = 'Unreal\Plugins\AutonomySim\Source\AutonomyLib'
    [System.IO.Directory]::CreateDirectory($AUTONOMYLIB_PLUGIN_DIR)
    Copy-Item -Path 'AutonomyLib' -Destination $AUTONOMYLIB_PLUGIN_DIR
    Copy-Item -Path 'AutonomySim.props' -Destination $AUTONOMYLIB_PLUGIN_DIR
}

function Get-VsUnrealProjectFiles {
    param(
        [Parameter(Mandatory=$true)]
        [String]
        $UnrealEnvDir
    )
    Set-Location $UnrealEnvDir
    Import-Module "$UnrealEnvDir\scripts\update_unreal_env.psm1"  # imports: Test-DirectoryPath, Copy-UnrealEnvItems, Remove-UnrealEnvItems, Invoke-VsUnrealProjectFileGenerator
    #Test-DirectoryPath -Path $PROJECT_DIR
    Copy-UnrealEnvItems -Path $PROJECT_DIR
    Remove-UnrealEnvItems
    Invoke-VsUnrealProjectFileGenerator
    Remove-Module "$UnrealEnvDir\scripts\update_unreal_env.psm1"
    Set-Location $PROJECT_DIR
}

function Update-VsUnrealProjectFiles {
    $UnrealEnvDirs = (Get-ChildItem -Path 'Unreal\Environments' -Directory | Select-Object FullName).FullName  # remove attribute decorator
    foreach ($UnrealEnvDir in $UnrealEnvDirs) {
        Get-VsUnrealProjectFiles -UnrealEnvDir $UnrealEnvDir
    }
}

###
### Main
###

if ( $DEBUG -eq $true ) { Write-Output (Get-WindowsInfo($SYSTEM_INFO)) }

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
Write-Output " Windows version:       $SYSTEM_OS_VERSION"
Write-Output " Visual Studio version: $VS_VERSION"
Write-Output " CMake version:         $CMAKE_VERSION"
Write-Output " RPClib version:        $RPCLIB_VERSION"
Write-Output " Eigen version:         $EIGEN_VERSION"
Write-Output " SUV asset version:     $ASSET_SUV_VERSION"
Write-Output " Full-polycount SUV:    $FULL_POLYCOUNT_SUV"
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output ''

# Ensure script is run from `AutonomySim` project directory
Test-WorkingDirectory

# Test Visual Studio version
Test-VisualStudioVersion

# Test CMake version
Test-CmakeVersion

# Create temporary build directories if they do not exist
Add-Directories

# Test RPClib version
Test-RpcLibVersion

# Test high-polycount SUV asset
Test-AssetSuvVersion

# Test Eigen library version
Test-EigenVersion

# Compile AutonomySim.sln including MavLinkCom
Build-Solution

# Copy binaries and includes for MavLinkCom and Unreal/Plugins
Copy-GeneratedBinaries

# Update all Unreal Engine environments under AutonomySim\Unreal\Environments
Update-VsUnrealProjectFiles

# Optionally build documentation
if ($BUILD_DOCS) { Build-Documentation }

exit 0