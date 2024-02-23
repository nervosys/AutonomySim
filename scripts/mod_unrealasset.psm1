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

# Common utilities
# imports: Add-Directories, Remove-Directories, Invoke-Fail, Test-WorkingDirectory, Test-VariableDefined,
#   Get-EnvVariables, Get-ProgramVersion, Get-VersionMajorMinor, Get-VersionMajorMinorBuild, Get-WindowsInfo,
#   Get-WindowsVersion, Get-Architecture, Get-ArchitectureWidth, Set-ProcessorCount
Import-Module "${PWD}\scripts\mod_utils.psm1"

###
### Variables
###

[String]$UNREAL_ASSET_VERSION = '1.2.0'
[String]$UNREAL_ASSET_URL = "https://github.com/Microsoft/AirSim/releases/download/v${UNREAL_ASSET_VERSION}/car_assets.zip"
[String]$ADVANCED_VEHICLE_DIR = '.\Unreal\Plugins\AutonomySim\Content\VehicleAdv'  # advanced vehicle template (AVT) directory

###
### Functions
###

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
  [System.Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12  # TLS v1.2
  Invoke-WebRequest "$UnrealAssetUrl" -OutFile '.\temp\car_assets.zip' -HttpVersion '2.0'
  # Unpack archive.
  Remove-Item "${AdvancedVehicleDir}\SUV" -Force -Recurse 2>$null
  Expand-Archive -Path '.\temp\car_assets.zip' -DestinationPath "$AdvancedVehicleDir"
  Remove-Item '.\temp\car_assets.zip' -Force -Recurse
  return $null
}

function Test-UnrealAssetVersion {
  param(
    [Parameter()]
    [Boolean]
    $FullPolySuv = $false,
    [Parameter()]
    [String]
    $UnrealAssetVersion = "$UNREAL_ASSET_VERSION",
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
Export-ModuleMember -Function Install-UnrealAsset, Test-UnrealAssetVersion
