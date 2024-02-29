<#
FILENAME:
  mod_visualstudio.psm1
DESCRIPTION:
  PowerShell script to validate Visual Studio version.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-20
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio >= 2019 (version 16).

  Copyright © 2024 Nervosys, LLC
#>

[String]$PROJECT_DIR = (Split-Path -Parent -Path (Split-Path -Parent -Path "$PSScriptRoot"))
[String]$SCRIPT_DIR = (Split-Path -Parent -Path "$PSScriptRoot")

###
### Imports
###

# Utilities
# imports: Add-Directories, Remove-TempDirectories, Invoke-Fail, Test-WorkingDirectory, Test-Program,
#   Test-VariableDefined, Get-EnvVariables, Get-ProgramVersion, Get-VersionMajorMinor,
#   Get-VersionMajorMinorBuild, Get-WindowsInfo, Get-WindowsVersion, Get-Architecture,
#   Get-ArchitectureWidth, Set-ProcessorCount
Import-Module "${SCRIPT_DIR}\mod_utils.psm1"

###
### Variables
###

[Version]$VS_VERSION_MINIMUM = '16.0'  # versions: [2019 = 16, 2022 = 17]
[Boolean]$AUTOMATE_MODE = $false

###
### Functions
###

function Get-VsInstance {
    [OutputType([PSCustomObject])]
    param(
        [Parameter()]
        [System.Object]
        $VsWhereArgs = "-all -sort"
    )
    $VsWherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $Expression = "& `"$VsWherePath`" $VsWhereArgs -format json"
    return Invoke-Expression $Expression | ConvertFrom-Json
}

function Set-VsInstance {
    [OutputType([PSCustomObject])]
    param(
        [Parameter()]
        [Boolean]
        $Automate = $AUTOMATE_MODE
    )
    [String]$SetupArgs = "-all -sort"
    $Configs = Get-VsInstance -VsWhereArgs $SetupArgs
    # Sort by version: highest to lowest
    $Configs = $Configs | Sort-Object installationVersion -Descending
    # Add ID number to each installation
    for ($i = 0; $i -lt $Configs.count; $i++) {
        $Configs[$i] | Add-Member -NotePropertyName "#" -NotePropertyValue "$i"
    }
    if ( $Verbose.IsPresent ) {
        $DisplayProperties = @('displayName', 'instanceId', 'installationVersion', 'isPrerelease', 'installationName', 'installDate')
        $DisplayProperties = @('#') + $DisplayProperties
        $VsVersionTable = ( $Configs | Format-Table -Property $DisplayProperties | Out-String | ForEach-Object { Write-Output $_ } )
        Write-Output 'The following Visual Studio installations were found:'
        Write-Output $VsVersionTable
    }
    # If automation is enabled: select the latest version
    if ( $Automate -eq $true ) {
        $Selected = '0'
    } elseif ( $Automate -eq $false ) {
        $Selected = Read-Host "Enter the '#' of the Visual Studio installation to use. Press <Enter> to quit: "
        if ( $Selected -eq '' ) { Invoke-Fail -ErrorMessage 'Error: Visual Studio instance not selected.' }
    } else {
        Write-Output 'No Visual Studio installation selected. Exiting program.'
        Invoke-Fail -ErrorMessage 'Error: Failed to select Visual Studio installation.'
    }
    $Config = ( $Configs | Where-Object { $_."#" -eq $Selected } )
    return $Config
}

function Get-VsInstanceVersion {
    [OutputType([Version])]
    param(
        [Parameter(Mandatory)]
        [System.Object]
        $Config  # object output by Get-VsInstance or Set-VsInstance
    )
    return [Version]::new($Config.installationVersion)
}

function Test-VsInstanceVersion {
    [OutputType()]
    param(
        [Parameter()]
        [Version]
        $MinimumVersion = $VS_VERSION_MINIMUM,
        [Parameter()]
        [Boolean]
        $Automate = $AUTOMATE_MODE
    )
    $VsInstance = Set-VsInstance -Automate $Automate
    $CurrentVersion = Get-VsInstanceVersion -Config $VsInstance
    if ( $null -eq $CurrentVersion ) {
        Invoke-Fail -ErrorMessage "Error: Failed to locate a Visual Studio instance."
    }
    if ( $CurrentVersion -lt $MinimumVersion ) {
        # install CMake if it is less than the required version
        Write-Output "$($Program) version $($CurrentVersion) is less than the minimum supported."
        Write-Output 'AutonomySim supports up to Unreal Engine 5.3 and Visual Studio 2022.'
        Write-Output 'Here are few easy steps to perform the upgrade:'
        Write-Output '  https://github.com/nervosys/AutonomySim/blob/master/docs/unreal_upgrade.md'
        Invoke-Fail -ErrorMessage "Error: Visual Studio version does not meet minimum requirement."
    } else {
        Write-Output "Success: Visual Studio version test passed."
    }
    return $null
}

###
### Exports
###

Export-ModuleMember -Variable VS_VERSION_MINIMUM
Export-ModuleMember -Function Get-VsInstance, Set-VsInstance, Get-VsInstanceVersion, Test-VsInstanceVersion
