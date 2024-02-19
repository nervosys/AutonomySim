<#
FILENAME:
  test_cmake.psm1
DESCRIPTION:
  PowerShell script to validate Eigen version.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  11-17-2023
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).
  Script is intended to run from AutonomySim base project directory.

  Copyright © 2024 Nervosys, LLC
#>

###
### Variables
###

$CMAKE_VERSION_MINIMUM = '3.14'
$CMAKE_VERSION_LATEST = '3.26.4'

###
### Functions
###

function Get-ProgramVersion {
    [OutputType([Version])]
    param(
        [Parameter(Mandatory = $true)]
        [String]
        $Program
    )
    return (Get-Command -Name $Program -ErrorAction SilentlyContinue).Version
}

function Get-VersionMajorMinor {
    [OutputType([String])]
    param(
        [Parameter(Mandatory = $true)]
        [Version]
        $Version
    )
    return $Version.Major, $Version.Minor -join '.'
}

function Get-VersionMajorMinorBuild {
    [OutputType([String])]
    param(
        [Parameter(Mandatory = $true)]
        [Version]
        $Version
    )
    return $Version.Major, $Version.Minor, $Version.Build -join '.'
}

function Install-Cmake {
    param(
        [Parameter()]
        [String]
        $Version = $CMAKE_VERSION_LATEST
    )
    Write-Host -NoNewLine "Download and install CMake v${Version}? [y|N]"
    $Response = [System.Console]::ReadKey().Key.ToString()  # uses automatic capitalization
    if ( $Response -eq 'Y' ) {
        $VersionMajMin = Get-VersionMajorMinor $Version
        $Installer = "cmake-${Version}-x86_64.msi"
        Invoke-WebRequest "https://cmake.org/files/v${VersionMajMin}/${Installer}" -OutFile "temp\${Installer}"
        Start-Process -FilePath "temp\${Installer}" -Wait
        Remove-Item -Path "temp\${Installer}"
    }
    else {
        Write-Error "Error: CMake version ${CMAKE_VERSION_MINIMUM} or greater is required, but was neither found nor installed." -ErrorAction Stop
    }
}

function Test-CmakeVersion {
    [OutputType([Boolean])]
    param(
        [Parameter()]
        [String]
        $MinimumVersion = $CMAKE_VERSION_MINIMUM,
        [Parameter()]
        [String]
        $Program = 'cmake'
    )
    $CurrentVersion = Get-ProgramVersion -Program $Program
    if ( $null -eq $CurrentVersion ) { Install-Cmake }  # install CMake if it is missing
    $RequiredVersion = [Version]$MinimumVersion
    if ($CurrentVersion -lt $RequiredVersion) {
        # install CMake if it is less than the required version
        Write-Error "Error: $($Program) version $($CurrentVersion) is less than the minimum supported." -ErrorAction SilentlyContinue
        Install-Cmake
    }
    else {
        Write-Output "Success: CMake version ${CurrentVersion} meets the minimum requirements."
    }
}

###
### Exports
###

Export-ModuleMember -Variable CMAKE_VERSION_MINIMUM
Export-ModuleMember -Function Test-CmakeVersion
