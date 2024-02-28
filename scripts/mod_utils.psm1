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

function Remove-ItemSilent {
  [OutputType()]
  param(
    [Parameter(HelpMessage = 'Path to the object for removal.')]
    [String]
    $Path,
    [Parameter(HelpMessage = 'Optional filters for pattern matching.')]
    [String]
    $Filter,
    [Parameter(HelpMessage = 'Optional switch for recursion.')]
    [Switch]
    $Recurse
  )
  if ( $Recurse.IsPresent ) {
    if ( $PSBoundParameters.ContainsKey('Filter') ) {
      Remove-Item -Path "$Path" -Filter "$Filter" -Recurse -Force -ErrorAction SilentlyContinue
    } else {
      Remove-Item -Path "$Path" -Recurse -Force -ErrorAction SilentlyContinue
    }
  } else {
    if ( $PSBoundParameters.ContainsKey('Filter') ) {
      Remove-Item -Path "$Path" -Filter "$Filter" -Force -ErrorAction SilentlyContinue
    } else {
      Remove-Item -Path "$Path" -Force -ErrorAction SilentlyContinue
    }
  }
  return $null
}

function Get-Modules {
  [OutputType([String])]
  param()
  return (Get-Module | Format-Table | Out-String)
}

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

function Test-DirectoryExists {
  [OutputType()]
  param(
      [Parameter()]
      [String]
      $Dir = "$PROJECT_DIR"
  )
  Write-Output "Testing directory path: ${Dir}"
  if ( -not (Test-Path -Path "$Dir") ) {
  	Write-Error "Error: Path not found." -ErrorAction Stop
  }
  return $null
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

function Remove-TempDirectories {
  [OutputType()]
  param(
    [Parameter()]
    [String[]]
    $Directories = @('temp', 'external')
  )
  foreach ($d in $Directories) {
    Remove-ItemSilent -Path "$d" -Recurse
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
    $RemoveTempDirs = $false,
    [Parameter()]
    [System.Exception]
    $Exception,
    [Parameter()]
    [String]
    $ErrorMessage
  )
  Set-Location "$ProjectDir"
  if ($RemoveTempDirs -eq $true) { Remove-TempDirectories }
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

function Get-EnvVariables {
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
  return ($Version.Major, $Version.Minor) -Join '.'
}

function Get-VersionMajorMinorBuild {
  [OutputType([String])]
  param(
    [Parameter(Mandatory)]
    [Version]
    $Version
  )
  return ($Version.Major, $Version.Minor, $Version.Build) -Join '.'
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

Export-ModuleMember -Function Remove-ItemSilent, Get-Modules, Test-VariableDefined, Test-DirectoryExists
Export-ModuleMember -Function Test-WorkingDirectory, Add-Directories, Remove-TempDirectories, Invoke-Fail
Export-ModuleMember -Function Test-VariableDefined, Get-EnvVariables, Test-Program, Get-ProgramVersion
Export-ModuleMember -Function Get-VersionMajorMinor, Get-VersionMajorMinorBuild, Get-WindowsInfo
Export-ModuleMember -Function Get-WindowsVersion, Get-Architecture, Get-ArchitectureWidth, Set-ProcessorCount
