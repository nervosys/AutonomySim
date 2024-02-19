<#
FILENAME:
  utils.psm1
DESCRIPTION:
  PowerShell utilities module.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  02-19-2024
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).

  Copyright © 2024 Nervosys, LLC
#>

###
### Functions
###

function Test-VariableDefined {
  param(
      [Parameter(Mandatory)]
      [String]
      $Variable
  )
  return [Boolean](Get-Variable $Variable -ErrorAction SilentlyContinue)
}

function Test-WorkingDirectory {
  $WorkingDirectory = Split-Path "$PWD" -Leaf
  if ($WorkingDirectory -ne 'AutonomySim') {
      Write-Output "Present working directory: ${PWD}"
      Write-Error "Error: Script must be run from 'AutonomySim' project directory." -ErrorAction Stop
  }
}

function Add-Directories {
  param(
      [Parameter()]
      [String[]]
      $Directories = @('temp', 'external', 'external\rpclib')
  )
  foreach ($d in $Directories) {
      [System.IO.Directory]::CreateDirectory("$d")
  }
}

function Remove-Directories {
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
    param(
        [Parameter()]
        [String]
        $ProjectDir = "$PWD",
        [Parameter()]
        [Switch]
        $RemoveDirs = $false,
        [Parameter()]
        [ErrorRecord]
        $ErrorCode,
        [Parameter()]
        [String]
        $ErrorMessage
    )
    Set-Location $ProjectDir
    if ($RemoveDirs -eq $true) { Remove-Directories }
    if (Test-VariableDefined -Variable $ErrorCode -eq $true) { Write-Error "Error code: ${ErrorCode}" -ErrorAction Continue }
    if (Test-VariableDefined -Variable $ErrorMessage -eq $true) { Write-Error "$ErrorMessage" -ErrorAction Continue }
    Write-Error 'Build failed. Exiting Program.' -ErrorAction Stop
}

function Get-EnvVariables {
  return Get-ChildItem 'env:*' | Sort-Object 'Name' | Format-List
}

function Get-ProgramVersion {
  [OutputType([Version])]
  param(
      [Parameter(Mandatory)]
      [String]
      $Program
  )
  return (Get-Command -Name $Program -ErrorAction SilentlyContinue).Version
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
  param(
      [Parameter(Mandatory)]
      [System.Object]
      $Info
  )
  return $Info | Select-Object WindowsProductName, WindowsVersion, OsHardwareAbstractionLayer
}

function Get-WindowsVersion {
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
  $arch = switch ($Info.CsSystemType) {
      'x64-based PC' { 'x64' }
      'x86-based PC' { 'x86' }
      $null { $null }
  }
  return $arch
}

function Get-ArchitectureWidth {
  [OutputType([String])]
  $archWidth = switch ([intptr]::Size) {
      4 { '32-bit' }
      8 { '64-bit' }
  }
  return $archWidth
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
