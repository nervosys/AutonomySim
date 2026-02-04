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
    
    # Check if any VS instances were found
    if ($null -eq $Configs -or $Configs.Count -eq 0) {
        Invoke-Fail -ErrorMessage "Error: No Visual Studio installations found. Please install Visual Studio 2019 or later."
    }
    
    # Convert single object to array if needed
    if ($Configs -isnot [array]) {
        $Configs = @($Configs)
    }
    
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
        Write-Output "Auto-selecting Visual Studio installation: $($Configs[0].displayName) (version $($Configs[0].installationVersion))"
    }
    elseif ( $Automate -eq $false ) {
        $Selected = Read-Host "Enter the '#' of the Visual Studio installation to use. Press <Enter> to quit: "
        if ( $Selected -eq '' ) { Invoke-Fail -ErrorMessage 'Error: Visual Studio instance not selected.' }
    }
    else {
        Write-Output 'No Visual Studio installation selected. Exiting program.'
        Invoke-Fail -ErrorMessage 'Error: Failed to select Visual Studio installation.'
    }
    $Config = ( $Configs | Where-Object { $_."#" -eq $Selected } )
    
    # Validate that a config was selected
    if ($null -eq $Config) {
        Invoke-Fail -ErrorMessage "Error: Invalid Visual Studio installation selection."
    }
    
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
    }
    else {
        Write-Output "Success: Visual Studio version test passed."
    }
    return $null
}

function Get-MsBuildPath {
    [OutputType([String])]
    param(
        [Parameter(Mandatory)]
        [System.Object]
        $Config  # object output by Get-VsInstance or Set-VsInstance
    )
    # MSBuild is located in the VS installation path
    $VsInstallPath = $Config.installationPath
    if ([string]::IsNullOrEmpty($VsInstallPath)) {
        Invoke-Fail -ErrorMessage "Error: Could not determine Visual Studio installation path."
    }
    
    # MSBuild path for VS 2019+
    $MsBuildPath = Join-Path $VsInstallPath "MSBuild\Current\Bin\MSBuild.exe"
    
    if (-not (Test-Path $MsBuildPath)) {
        # Try alternative path for older VS versions
        $MsBuildPath = Join-Path $VsInstallPath "MSBuild\15.0\Bin\MSBuild.exe"
    }
    
    if (-not (Test-Path $MsBuildPath)) {
        Invoke-Fail -ErrorMessage "Error: Could not locate MSBuild.exe in Visual Studio installation at: $VsInstallPath"
    }
    
    return $MsBuildPath
}

###
### Exports
###

Export-ModuleMember -Variable VS_VERSION_MINIMUM
Export-ModuleMember -Function Get-VsInstance, Set-VsInstance, Get-VsInstanceVersion, Test-VsInstanceVersion, Get-MsBuildPath
