<#
FILENAME:
  mod_unrealasset.psm1
DESCRIPTION:
  PowerShell script to validate Unreal Engine SUV asset version.
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

# Utilities
# imports: Add-Directories, Remove-Directories, Invoke-Fail, Test-WorkingDirectory, Test-VariableDefined,
#   Get-EnvVariables, Get-ProgramVersion, Get-VersionMajorMinor, Get-VersionMajorMinorBuild, Get-WindowsInfo,
#   Get-WindowsVersion, Get-Architecture, Get-ArchitectureWidth, Set-ProcessorCount
Import-Module "${PWD}\scripts\mod_utils.psm1"

# Unreal Environments
# imports: Get-UnrealVersion, Copy-UnrealEnvItems, Restore-UnrealEnv, Invoke-UnrealVsProjectFileGenerator
Import-Module "${PWD}\scripts\mod_unreal_env.psm1"

###
### Variables
###

[String]$PROJECT_DIR = "$PWD"

[Version]$UNREAL_VERSION = '5.3'

[Version]$UNREAL_ASSET_VERSION = '1.2.0'
[String]$UNREAL_ASSET_VERSION_STRING = Get-VersionMajorMinorBuild -Version $UNREAL_ASSET_VERSION
[String]$UNREAL_ASSET_URL = "https://github.com/Microsoft/AirSim/releases/download/v${UNREAL_ASSET_VERSION_STRING}/car_assets.zip"

# advanced vehicle template (AVT) directory
[String]$ADVANCED_VEHICLE_DIR = "${PROJECT_DIR}\UnrealPlugin\Unreal\Plugins\AutonomySim\Content\VehicleAdv"
[String]$UNREAL_ENV_DIR = "${PROJECT_DIR}\UnrealPlugin\Unreal\Environments\Blocks"

###
### Functions
###

function Get-UnrealVsProjectFiles {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $ProjectDir = "$PROJECT_DIR",
    [Parameter()]
    [String]
    $UnrealEnvDir = "$UNREAL_ENV_DIR",
    [Parameter()]
    [Version]
    $UnrealVersion = $UNREAL_VERSION,
    [Parameter()]
    [Boolean]
    $Automate = $true
  )
  # Set-Location "$UnrealEnvDir"
  # imports: Test-DirectoryPath, Copy-UnrealEnvItems, Remove-UnrealEnvItems, Invoke-VsUnrealProjectFileGenerator
  # Import-Module "${UnrealEnvDir}\scripts\update_unreal_env.psm1"
  # Test-DirectoryPath -Path $ProjectDir
  Copy-UnrealEnvItems -ProjectDir "$ProjectDir" -UnrealEnvDir "$UnrealEnvDir"
  Restore-UnrealEnv -UnrealEnvDir "$UnrealEnvDir"
  Invoke-UnrealVsProjectFileGenerator -UnrealVersion $UnrealVersion -Automate $Automate
  # Remove-Module -Name 'update_unreal_env'  # Get-Module
  # Set-Location "$ProjectDir"
  return $null
}

function Update-UnrealVsProjectFiles {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $ProjectDir = "$PROJECT_DIR",
    [Parameter()]
    [String]
    $UnrealEnvRootDir = "${PROJECT_DIR}\UnrealPlugin\Unreal\Environments",
    [Parameter()]
    [Version]
    $UnrealVersion = $UNREAL_VERSION,
    [Parameter()]
    [Boolean]
    $Automate = $true
  )
  $UnrealEnvDirs = (Get-ChildItem -Path "$UnrealEnvBaseDir" -Directory | Select-Object FullName).FullName
  foreach ( $d in $UnrealEnvDirs ) {
    Get-UnrealVsProjectFiles -UnrealEnvDir "$d" -ProjectDir "$ProjectDir" -UnrealVersion $UnrealVersion -Automate $Automate
  }
  return $null
}

function Install-UnrealAsset {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $UnrealAssetUrl = "$UNREAL_ASSET_URL",
    [Parameter()]
    [String]
    $AdvancedVehicleDir = "$ADVANCED_VEHICLE_DIR"
  )
  if ( $Verbose.IsPresent ) {
    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output ' Downloading 37-MB Unreal high-polycount SUV asset...'
    Write-Output '-----------------------------------------------------------------------------------------'
  }
  Remove-Item -Path '.\temp\car_assets.zip' -Force 2>$null
  [System.Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12  # TLS v1.2
  Invoke-WebRequest "$UnrealAssetUrl" -OutFile '.\temp\car_assets.zip' -HttpVersion '2.0'
  # Unpack archive.
  Remove-Item "${AdvancedVehicleDir}\SUV" -Force -Recurse 2>$null
  Expand-Archive -Path '.\temp\car_assets.zip' -DestinationPath "$AdvancedVehicleDir"
  Remove-Item -Path '.\temp\car_assets.zip' -Force -Recurse 2>$null
  return $null
}

function Test-UnrealAssetVersion {
  param(
    [Parameter()]
    [Boolean]
    $FullPolySuv = $false,
    [Parameter()]
    [String]
    $UnrealAssetVersion = "$UNREAL_ASSET_VERSION_STRING",
    [Parameter()]
    [String]
    $UnrealAssetUrl = "$UNREAL_ASSET_URL",
    [Parameter()]
    [String]
    $AdvancedVehicleDir = "$ADVANCED_VEHICLE_DIR"
  )
  if ( $FullPolySuv ) {
    if ( -not (Test-Path -LiteralPath "${AdvancedVehicleDir}\SUV\v${UnrealAssetVersion}") ) {
      # Create advanced vehicle template directory if it does not exist.
      New-Item -ItemType Directory -Path "$AdvancedVehicleDir" -Force | Out-Null
      # Download SUV asset.
      Install-UnrealAsset -UnrealAssetUrl "$UnrealAssetUrl" -AdvancedVehicleDir "$AdvancedVehicleDir"
      Write-Output 'Success: Unreal asset version test passed.'
      # If high-polycount SUV is unable to download, notify users that gokart will be used.
      if ( -not (Test-Path -LiteralPath "${AdvancedVehicleDir}\SUV") ) {
        Write-Output 'Download of high-polycount SUV failed. AutonomySim will use the default vehicle.'
        Invoke-Fail -ErrorMessage "Error: Failed to download and unpack high-polygon-count SUV asset."
      }
    }
    else {
      Write-Output 'Success: Unreal asset version test passed.'
    }
  }
  else {
    Write-Output 'Skipped: Download of Unreal high-polycount SUV asset. Default vehicle will be used.'
  }
  return $null
}

###
### Exports
###

Export-ModuleMember -Variable UNREAL_ASSET_VERSION
Export-ModuleMember -Function Get-UnrealVsProjectFiles, Update-UnrealVsProjectFiles
Export-ModuleMember -Function Install-UnrealAsset, Test-UnrealAssetVersion
