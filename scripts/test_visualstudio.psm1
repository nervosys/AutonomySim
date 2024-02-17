<#
FILENAME:
  test_visualstudio.psm1
DESCRIPTION:
  PowerShell script to validate VS version.
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

$VS_VERSION_MINIMUM = '17.0'

###
### Functions
###

function Get-VsInstance {
    param(
        [Parameter()]
        [System.Object]
        $VsWhereArgs
    )
    $VsWherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $expression = "& `"$VsWherePath`" $VsWhereArgs -format json"
    Invoke-Expression $expression | ConvertFrom-Json
}

function Set-VsInstance {
    $setupargs = "-all -sort"
    $configs = Get-VsInstance -VsWhereArgs $setupargs
    $DisplayProperties = @("displayName", "instanceId", "installationVersion", "isPrerelease", "installationName", "installDate")
    $DisplayProperties = @("#") + $DisplayProperties
    # Add an incrementing select column
    $configs = $configs |
    Sort-Object displayName, installationDate |
    ForEach-Object { $i = 0; $i++; $_ | Add-Member -NotePropertyName "#" -NotePropertyValue $i -PassThru }
    Write-Output "The following Visual Studio installations were found:"
    $configs | Format-Table -Property $DisplayProperties | Out-String | ForEach-Object { Write-Host $_ }
    $selected = Read-Host "Enter the '#' of the Visual Studio installation to use. Press <Enter> to quit: "
    if (-not $selected) {
        Write-Output "No Visual Studio installation selected. Exiting program."
        Invoke-Fail
    }
    $config = $configs | Where-Object { $_."#" -eq $selected }
    return $config
}

function Get-VsInstanceVersion {
    [OutputType([Version])]
    param(
        [Parameter(Mandatory)]
        [System.Object]
        $Config  # configuration output by Get-VsInstance or Set-VsInstance
    )
    return [Version]::new($Config.installationVersion)
}

function Test-VisualStudioVersion {
    [OutputType([Boolean])]
    param(
        # Parameter help description
        [Parameter()]
        [String]
        $MinimumVersion = $VS_VERSION_MINIMUM
    )
    $VsInstance = Set-VsInstance
    $CurrentVersion = Get-VsInstanceVersion($VsInstance)
    if ( $null -eq $CurrentVersion ) {
        Write-Error 'Error: Failed to locate a Visual Studio instance.'
        Invoke-Fail
    }
    $RequiredVersion = [Version]$MinimumVersion
    if ($CurrentVersion -lt $RequiredVersion) {
        # install CMake if it is less than the required version
        Write-Output ''
        Write-Output "$($Program) version $($CurrentVersion) is less than the minimum supported."
        Write-Output 'AutonomySim supports up to Unreal Engine 5.3 and Visual Studio 2022.'
        Write-Output 'Here are few easy steps to perform the upgrade:'
        Write-Output '  https://github.com/nervosys/AutonomySim/blob/master/docs/unreal_upgrade.md'
        Invoke-Fail
    }
    else {
        Write-Output "Success: Visual Studio version ${CurrentVersion} meets the minimum requirements."
    }
}

###
### Exports
###

Export-ModuleMember -Variable VS_VERSION_MINIMUM
Export-ModuleMember -Function Set-VsInstance
Export-ModuleMember -Function Get-VsInstanceVersion
Export-ModuleMember -Function Test-VisualStudioVersion
