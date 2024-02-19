<#
FILENAME:
  test_cmake.psm1
DESCRIPTION:
  PowerShell script to validate Eigen version.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  02-19-2024
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).
  
  Copyright © 2024 Nervosys, LLC
#>

###
### Imports
###

# Common utilities:
#   Add-Directories, Remove-Directories, Invoke-Fail, Test-WorkingDirectory, Test-VariableDefined,
#   Get-EnvVariables, Get-ProgramVersion, Get-VersionMajorMinor, Get-VersionMajorMinorBuild,
#   Get-WindowsInfo, Get-WindowsVersion, Get-Architecture, Get-ArchitectureWidth, Set-ProcessorCount
Import-Module "${PWD}\scripts\utils.psm1"

###
### Variables
###

[Version]$CMAKE_VERSION_MINIMUM = '3.14'
[Version]$CMAKE_VERSION_LATEST = '3.26.4'

###
### Functions
###

function Install-Cmake {
    param(
        [Parameter()]
        [Version]
        $Version = $CMAKE_VERSION_LATEST
    )
    Write-Host -NoNewLine "Download and install CMake v${Version}? [y|N]"
    $Response = [System.Console]::ReadKey().Key.ToString()  # uses automatic capitalization
    if ( $Response -eq 'Y' ) {
        $VersionMajMin = Get-VersionMajorMinor $Version
        $VersionMajMinBuild = Get-VersionMajorMinorBuild $Version
        $Installer = "cmake-${VersionMajMinBuild}-x86_64.msi"
        Invoke-WebRequest "https://cmake.org/files/v${VersionMajMin}/${Installer}" -OutFile "temp\${Installer}"
        Start-Process -FilePath "temp\${Installer}" -Wait
        Remove-Item -Path "temp\${Installer}"
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
    $CurrentVersion = Get-ProgramVersion -Program $Program
    if ( $null -eq $CurrentVersion ) { Install-Cmake }  # install CMake if it is missing
    if ($CurrentVersion -lt $MinimumVersion) {
        # install CMake if it is less than the required version
        Write-Error "Error: $($Program) version $($CurrentVersion) is less than the minimum supported." -ErrorAction SilentlyContinue
        Install-Cmake
    } else {
        Write-Output "Success: CMake version ${CurrentVersion} meets the minimum requirements."
    }
}

###
### Exports
###

Export-ModuleMember -Variable CMAKE_VERSION_MINIMUM
Export-ModuleMember -Function Test-CmakeVersion
