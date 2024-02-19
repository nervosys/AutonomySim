<#
FILENAME:
  test_visualstudio.psm1
DESCRIPTION:
  PowerShell script to validate VS version.
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

[Version]$VS_VERSION_MINIMUM = '16.0'  # versions: [2019 = 16, 2022 = 17]

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
        Invoke-Fail -ErrorMessage "Error: Failed to select Visual Studio installation."
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
        [Version]
        $MinimumVersion = $VS_VERSION_MINIMUM
    )
    $VsInstance = Set-VsInstance
    $CurrentVersion = Get-VsInstanceVersion($VsInstance)
    if ( $null -eq $CurrentVersion ) {
        Invoke-Fail -ErrorMessage "Error: Failed to locate a Visual Studio instance."
    }
    if ($CurrentVersion -lt $MinimumVersion) {
        # install CMake if it is less than the required version
        Write-Output ''
        Write-Output "$($Program) version $($CurrentVersion) is less than the minimum supported."
        Write-Output 'AutonomySim supports up to Unreal Engine 5.3 and Visual Studio 2022.'
        Write-Output 'Here are few easy steps to perform the upgrade:'
        Write-Output '  https://github.com/nervosys/AutonomySim/blob/master/docs/unreal_upgrade.md'
        Invoke-Fail -ErrorMessage "Error: Visual Studio version does not meet minimum requirement."
    }
    else {
        Write-Output "Success: Visual Studio version ${CurrentVersion} meets the minimum requirements."
    }
}

###
### Exports
###

Export-ModuleMember -Variable VS_VERSION_MINIMUM
Export-ModuleMember -Function Set-VsInstance, Get-VsInstanceVersion, Test-VisualStudioVersion
