<#
FILENAME:
  mod_utils.psm1
DESCRIPTION:
  PowerShell utilities module.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-22
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).

  Copyright © 2024 Nervosys, LLC
#>

###
### Functions
###

function Test-VariableDefined {
  [OutputType([Boolean])]
  param(
    [Parameter(Mandatory)]
    [String]
    $VariableName
  )
  [Boolean]$Exists = Test-Path "Variable:\$VariableName"
  return $Exists
}

function Test-WorkingDirectory {
  [OutputType()]
  param()
  $WorkingDirectory = Split-Path "$PWD" -Leaf
  if ($WorkingDirectory -ne 'AutonomySim') {
    Write-Output "Present working directory: ${PWD}"
    Write-Error "Error: Script must be run from 'AutonomySim' project directory." -ErrorAction Stop
  }
}

function Add-Directories {
  [OutputType()]
  param(
    [Parameter()]
    [String[]]
    $Directories = @('temp', 'external', 'external\rpclib')
  )
  foreach ($d in $Directories) {
    [System.IO.Directory]::CreateDirectory("$d") | Out-Null
  }
}

function Remove-Directories {
  [OutputType()]
  param(
    [Parameter()]
    [String[]]
    $Directories = @('temp', 'external')
  )
  foreach ($d in $Directories) {
    Remove-Item -Path "$d" -Force -Recurse
  }
}

function Invoke-Fail {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $ProjectDir = "$PWD",
    [Parameter()]
    [Switch]
    $RemoveDirs = $false,
    [Parameter()]
    [System.Exception]
    $Exception,
    [Parameter()]
    [String]
    $ErrorMessage
  )
  Set-Location "$ProjectDir"
  if ($RemoveDirs -eq $true) { Remove-Directories }
  if ((Test-VariableDefined -VariableName "ErrorMessage") -eq $true) {
    Write-Error -Exception [System.Exception] -Message "$ErrorMessage" -ErrorAction Continue
  }
  if ((Test-VariableDefined -VariableName "Exception") -eq $true) {
    throw $Exception
  }
}

function Get-Exceptions {
  [OutputType([String[]])]
  [String[]]$Exceptions = [AppDomain]::CurrentDomain.GetAssemblies() | foreach {
    try {
      $_.GetExportedTypes().BaseType | Where { $_.Fullname -Match 'Exception' }
    } catch {}
  }
  return $Exceptions | Sort-Object -Unique
}

function Get-EnvVars {
  [OutputType([Object[]])]
  [Object[]]$EnvVars = Get-ChildItem 'env:*' | Sort-Object 'Name' | Format-List
  return $EnvVars
}

function Test-Program {
  [OutputType([Boolean])]
  param(
    [Parameter(Mandatory)]
    [String]
    $Program
  )
  $ProgramResult = (Get-Command -Name "$Program" -ErrorAction SilentlyContinue)
  [Boolean]$ProgramExists = if ( $null -eq $ProgramResult ) { $false } else { $true }
  return $ProgramExists
}

function Get-ProgramVersion {
  [OutputType([Version])]
  param(
    [Parameter(Mandatory)]
    [String]
    $Program
  )
  return (Get-Command -Name "$Program" -ErrorAction SilentlyContinue).Version
}

function Get-VersionMajorMinor {
  [OutputType([String])]
  param(
    [Parameter(Mandatory)]
    [Version]
    $Version
  )
  return $Version.Major, $Version.Minor -join '.'
}

function Get-VersionMajorMinorBuild {
  [OutputType([String])]
  param(
    [Parameter(Mandatory)]
    [Version]
    $Version
  )
  return $Version.Major, $Version.Minor, $Version.Build -join '.'
}

function Get-WindowsInfo {
  [OutputType([PSCustomObject])]
  param(
    [Parameter(Mandatory)]
    [System.Object]
    $Info
  )
  [PSCustomObject]$SystemInfo = $Info | Select-Object WindowsProductName, WindowsVersion, OsHardwareAbstractionLayer
  return $SystemInfo
}

function Get-WindowsVersion {
  [OutputType([Version])]
  param(
    [Parameter(Mandatory)]
    [System.Object]
    $Info
  )
  return [Version]$Info.OsHardwareAbstractionLayer
}

function Get-Architecture {
  [OutputType([String])]
  param(
    [Parameter(Mandatory)]
    [System.Object]
    $Info
  )
  $Arch = switch ($Info.CsSystemType) {
    'x64-based PC' { 'x64' }
    'x86-based PC' { 'x86' }
    $null { $null }
  }
  return $Arch
}

function Get-ArchitectureWidth {
  [OutputType([String])]
  $ArchWidth = switch ([intptr]::Size) {
    4 { '32-bit' }
    8 { '64-bit' }
  }
  return $ArchWidth
}

function Set-ProcessorCount {
  [OutputType([UInt32])]
  param(
    [Parameter(Mandatory)]
    [System.Object]
    $Info,
    [Parameter(HelpMessage = 'The number of processor cores remaining. Use all others for MSBuild.')]
    [UInt32]
    $Remainder = 2
  )
  return [UInt32]$Info.CsNumberOfLogicalProcessors - $Remainder
}

###
### Exports
###

Export-ModuleMember -Function Add-Directories, Remove-Directories, Invoke-Fail, Test-WorkingDirectory, Test-VariableDefined
Export-ModuleMember -Function Get-EnvVariables, Get-ProgramVersion, Get-VersionMajorMinor, Get-VersionMajorMinorBuild
Export-ModuleMember -Function Get-WindowsInfo, Get-WindowsVersion, Get-Architecture, Get-ArchitectureWidth, Set-ProcessorCount
