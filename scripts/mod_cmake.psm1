<#
FILENAME:
  mod_cmake.psm1
DESCRIPTION:
  PowerShell script to validate Eigen version.
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

[Version]$CMAKE_VERSION = '3.29.5'
[Version]$CMAKE_VERSION_MINIMUM = '3.14'

###
### Functions
###

function Install-Cmake {
  param(
    [Parameter()]
    [Version]
    $Version = $CMAKE_VERSION,
    [Parameter()]
    [Switch]
    $Automate
  )
  if ( $Automate.IsPresent ) { $Response = 'N' }
  else {
    Write-Host -NoNewLine "Download and install CMake v${Version}? [y|N]"
    $Response = [System.Console]::ReadKey().Key.ToString()  # uses automatic capitalization
  }
  # case insensitive
  if ( $Response -eq 'Y' ) {
    if ( $Verbose.IsPresent ) {
      Write-Output '-----------------------------------------------------------------------------------------'
      Write-Output ' Downloading and installing CMake...'
      Write-Output '-----------------------------------------------------------------------------------------'
    }
    $VersionMajMin = Get-VersionMajorMinor $Version
    $VersionMajMinBuild = Get-VersionMajorMinorBuild $Version
    $Installer = "cmake-${VersionMajMinBuild}-x86_64.msi"
    Invoke-WebRequest "https://cmake.org/files/v${VersionMajMin}/${Installer}" -OutFile ".\temp\${Installer}"
    Start-Process -FilePath ".\temp\${Installer}" -Wait -NoNewWindow
    Remove-ItemSilent -Path ".\temp\${Installer}"
  } else {
    Write-Error "Error: CMake version ${CMAKE_VERSION_MINIMUM} or greater is required, but was neither found nor installed." -ErrorAction Continue
    Invoke-Fail -ErrorMessage "Error: Failed to install CMake." 
  }
}

function Test-CmakeVersion {
  [OutputType([Boolean])]
  param(
    [Parameter()]
    [Version]
    $MinimumVersion = $CMAKE_VERSION_MINIMUM,
    [Parameter()]
    [String]
    $Program = 'cmake'
  )
  $CurrentVersion = Get-ProgramVersion -Program "$Program"
  if ( $null -eq $CurrentVersion ) { Install-Cmake }  # install CMake if it is missing
  if ($CurrentVersion -lt $MinimumVersion) {
    # install CMake if it is less than the required version
    Write-Error "Error: $($Program) version $($CurrentVersion) is less than the minimum supported." -ErrorAction SilentlyContinue
    Install-Cmake
  } else {
    Write-Output "Success: CMake version test passed."
  }
}

###
### Exports
###

Export-ModuleMember -Variable CMAKE_VERSION
Export-ModuleMember -Function Install-Cmake, Test-CmakeVersion
