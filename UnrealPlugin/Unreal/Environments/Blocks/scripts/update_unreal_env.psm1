<#
FILENAME:
  update_unreal_env.psm1
DESCRIPTION:
  PowerShell script to update Unreal Engine environment.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-27
NOTES:
  - Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).
  - Script is intended to run from `.\AutonomySim\UnrealPlugin\Unreal\Environments\<Environment>\`
    directory, which we invoke from `.\AutonomySim\scripts\build.ps1`.
USAGE:
  `powershell .\AutonomySim\UnrealPlugin\Unreal\Environments\<Environment>\update_unreal_env.psm1`

  Copyright © 2024 Nervosys, LLC
#>

###
### Imports
###

# Utilities
Import-Module "${PWD}\scripts\mod_utils.psm1"

###
### Variables
###

[String]$PROJECT_DIR = "$PWD"
[Version]$UNREAL_VERSION = '5.3'

###
### Functions
###

function Test-ProjectDirPath {
  [OutputType()]
  param(
      [Parameter()]
      [String]
      $ProjectDir = "$PROJECT_DIR"
  )
  Write-Output "Testing directory path: ${ProjectDir}"
  if ( -not (Test-Path -Path "$ProjectDir") ) {
  	Write-Error "Error: Path not found." -ErrorAction Stop
  }
  return $null
}

function Get-UnrealVersion {
  [OutputType([String])]
  param(
    [Parameter()]
    [Version]
    $UnrealVersion = $UNREAL_VERSION
  )
  if ($UnrealVersion.Build -eq -1) {
    [String]$Result = ($UnrealVersion.Major, $UnrealVersion.Minor) -Join '.'
  } else {
    [String]$Result = ($UnrealVersion.Major, $UnrealVersion.Minor, $UnrealVersion.Build) -Join '.'
  }
  return $Result
}

function Copy-UnrealEnvItems {
  [OutputType()]
  param(
      [Parameter()]
      [String]
      $ProjectDir = "$PROJECT_DIR",
      [Parameter()]
      [String]
      $UnrealEnvDir = "${PROJECT_DIR}\UnrealPlugin\Unreal\Environments\Blocks"
  )
  # Copy generated VS project files
  # NOTE:
  #   - Ensure all files in source directory make it to destination directory
  #   - Remove all destination files not present in source directory
  # Robust Copy (robocopy) flags:
  #   *.:  all files
  #   MIR: mirror directory
  #   XD:  exclude directory
  #   NJH: no job header
  #   NJS: no job summary
  #   NDL: no directory listing
  #   NP:  no progress
  #robocopy /MIR "$AUTONOMYSIM_PATH\Unreal\Plugins\AutonomySim" 'Plugins\AutonomySim' /XD 'temp' *. /NJH /NJS /NDL /NP
  #robocopy /MIR "$AUTONOMYSIM_PATH\AutonomyLib" 'Plugins\AutonomySim\Source\AutonomyLib' /XD 'temp' *. /NJH /NJS /NDL /NP
  #robocopy  /NJH /NJS /NDL /NP "$AUTONOMYSIM_PATH\Unreal\Environments\Blocks" '.' *.cmd 
  #robocopy  /NJH /NJS /NDL /NP "$AUTONOMYSIM_PATH\Unreal\Environments\Blocks" '.' *.sh 
  Copy-Item -Path "${ProjectDir}\UnrealPlugin\Unreal\Plugins\AutonomySim\*" -Destination '.\Plugins\AutonomySim' -Exclude @("temp") -Recurse -Force
  Copy-Item -Path "${ProjectDir}\AutonomyLib" -Destination '.\Plugins\AutonomySim\Source\AutonomyLib' -Exclude @("temp") -Recurse -Force
  Copy-Item -Path "${UnrealEnvDir}\*.cmd" -Destination '.' -Recurse -Force
  Copy-Item -Path "${UnrealEnvDir}\*.sh" -Destination '.' -Recurse -Force
  return $null
}

function Restore-UnrealEnv {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $ProjectDir = "$PROJECT_DIR",
    [Parameter()]
    [String]
    $UnrealEnvDir = "${PROJECT_DIR}\UnrealPlugin\Unreal\Environments\Blocks"
  )
  Remove-ItemSilent -Path "${UnrealEnvDir}\Build" -Recurse
  Remove-ItemSilent -Path "${UnrealEnvDir}\Binaries" -Recurse
  Remove-ItemSilent -Path "${UnrealEnvDir}\Intermediate" -Recurse
  Remove-ItemSilent -Path "${UnrealEnvDir}\Saved" -Recurse
  [System.IO.Directory]::CreateDirectory("${UnrealEnvDir}\Saved\logs") | Out-Null
  Remove-ItemSilent -Path "${UnrealEnvDir}\Plugins\AutonomySim\Binaries" -Recurse
  Remove-ItemSilent -Path "${UnrealEnvDir}\Plugins\AutonomySim\Intermediate" -Recurse
  Remove-ItemSilent -Path "${UnrealEnvDir}\Plugins\AutonomySim\Saved" -Recurse
  Remove-ItemSilent -Path "$UnrealEnvDir" -Filter '*.sln' -Recurse
  return $null
}

function Invoke-UnrealVsProjectFileGenerator {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $UnrealVersion = (Get-UnrealVersion -UnrealVersion $UNREAL_VERSION),
    [Parameter()]
    [Boolean]
    $Automate = $true
  )
  # Visual Studio Unreal Engine Project File Generator
  #   Remove-Item -Path 'gen_temp.txt'
  # Get absolute path to Unreal Version Selector from `rungenproj` registry entry and save to single-line text file.
  # The `rungenproj` program, `UnrealVersionSelector.exe`, generates Visual Studio project files.
  # NOTE: Set Out-File (>) encoding to ASCII for consistency with older Set-Content command
  if ( $Automate ) {
    [String]$Generator = "C:\Program Files\Epic Games\UE_${UnrealVersion}\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"
  } else {
    # Default on Windows 10/11 (requires surrounding quotation marks)
    [String]$Generator = '"C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe"'
    # Registry Key path to UnrealVersionSelector.exe
    # [String]$GeneratorRegistryKey = 'Registry::HKEY_CLASSES_ROOT\Unreal.ProjectFile\shell\rungenproj'
    # [String]$Generator = (Get-ItemProperty $GeneratorRegistryKey -Name 'Icon').Icon
  }
  # (Get-ItemProperty 'Registry::HKEY_CLASSES_ROOT\Unreal.ProjectFile\shell\rungenproj' -Name 'Icon').Icon |
  #   Out-File -Encoding 'ascii' -FilePath 'gen_temp.txt'
  # $gen_bin = Get-Content 'gen_temp.txt' -Encoding 'ascii' -ReadCount 1  # `UnrealVersionSelector.exe` path
  # Remove-Item -Path 'gen_temp.txt'  # remove temporary file
  # $gen_bin /projectfiles "$PWD\$Project"
  $ProjectFiles = (Get-ChildItem -Path *.uproject -File).FullName
  foreach ( $ProjectFile in $ProjectFiles ) {
    [String]$ProjectName = [System.IO.Path]::GetFileNameWithoutExtension("$ProjectFile")
    Write-Output "Generating Visual Studio build files: ${ProjectName}"
    if ( $Automate ) {
      Start-Process -FilePath "$Generator" -ArgumentList '-projectfiles',"-project=${ProjectFile}",'-progress' -Wait -NoNewWindow
    } else {
      Start-Process -FilePath "$Generator" -ArgumentList '/projectfiles',"${ProjectFile}" -Wait -NoNewWindow
    }
  }
  return $null
}

###
### Exports
###

Export-ModuleMember -Function Get-UnrealVersion, Copy-UnrealEnvItems, Restore-UnrealEnv
Export-ModuleMember -Function Invoke-UnrealVsProjectFileGenerator
