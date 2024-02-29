<#
FILENAME:
  mod_format.psm1
DESCRIPTION:
  PowerShell module for formatting scripts or directories.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-20
NOTES:
  Assumes: PowerShell version >= 7.
  Installs: PSScriptAnalyzer module.
  
  Copyright © 2024 Nervosys, LLC
#>

###
### Variables
###

[String]$PROJECT_DIR = (Split-Path -Parent -Path (Split-Path -Parent -Path "$PSScriptRoot"))
[String]$SCRIPT_DIR = (Split-Path -Parent -Path "$PSScriptRoot")

# Specify PowerShell script directories to avoid scanning the entire repository.
$PS_DIR_PATHS = @("$SCRIPT_DIR")

$PS_FILES_INCLUDE = @('*.ps1', '*.psm1', '*.psd1')
$PS_FILES_EXCLUDE = @()

# Specify C/C++ project directories to avoid scanning the entire repository.
$CPP_DIR_PATHS = @(
  'AutonomyLib', 'AutonomyLibUnitTests', 'DroneServer', 'DroneShell', 'Examples', 'HelloCar',
  'HelloDrone', 'HelloSpawnedDrones', 'MavLinkCom', 'ros2', 'SemiGlobalMatching', 'UnrealPlugin'
)
$CPP_DIR_PATHS = $CPP_DIR_PATHS | % { Join-Path $PROJECT_DIR $_ }

$CPP_FILES_INCLUDE = @('*.c', '*.cpp', '*.cc', '*.hpp', '*.h')
$CPP_FILES_EXCLUDE = @('json.hpp')

$CPP_FORMAT_EXE = (Get-Command 'clang-format.exe' -ErrorAction Stop).Source
$CPP_FORMAT_CFG = (Resolve-Path -Path '.clang-format' -ErrorAction Stop).Path
# $CPP_FORMAT_LOG = ".\format_cpp.log"

###
### Functions
###

function Install-ModuleIfMissing {
  [OutputType()]
  param(
    [Parameter(Mandatory, HelpMessage = 'Name of the PowerShell module.')]
    [String]
    $ModuleName
  )
  if ( $Verbose.IsPresent ) { Write-Output 'Checking if moduled is installed...' }
  if ( -not (Get-Module -Name "$ModuleName" -ListAvailable | Select-Object Name, Version) ) {
    if ( $Verbose.IsPresent ) {
      Write-Output "Module not found: ${ModuleName}"
      Write-Output "Installing module: ${ModuleName}"
    }
    Install-Module -Name "$ModuleName" -Force -Confirm:$false
  }
  else {
    if ( $Verbose.IsPresent ) { Write-Output "Module found: ${ModuleName}" }
  }
  return $null
}

function Get-CppFiles {
  [OutputType([String[]])]
  param(
    [Parameter(Mandatory, HelpMessage = 'Path to PowerShell script or directory for formatting.')]
    [String]
    $Path,
    [Parameter(HelpMessage = 'File patterns to include.')]
    [String[]]
    $Include = $CPP_FILES_INCLUDE,
    [Parameter(HelpMessage = 'File patterns to exclude.')]
    [String[]]
    $Exclude = $CPP_FILES_EXCLUDE
  )
  return (Get-ChildItem -Path "$Path" -File -Include $Include -Exclude $Exclude -Recurse | Select-Object -ExpandProperty FullName)
}

function Format-CppRecursive {
  [OutputType()]
  param (
    [Parameter(HelpMessage = 'Array of paths to C/C++ source code directories.')]
    [String[]]
    $Paths = $CPP_DIR_PATHS,
    [Parameter(HelpMessage = 'Path to clang-format executable file.')]
    [String]
    $ClangFormat = $CPP_FORMAT_EXE,
    [Parameter(HelpMessage = 'Path to clang-format style configuration file.')]
    [String]
    $ConfigFile = $CPP_FORMAT_CFG
  )
  if ( $Verbose.IsPresent ) {
    Write-Output "Found clang-format: ${CLANG_FORMAT_EXE}"
    Write-Output "Found clang-format configuration: ${CLANG_FORMAT_CFG}"
    Write-Output "Formatting C/C++ files..."
  }
  foreach ( $d in $Paths ) {
    [String[]]$FilePaths = Get-CppFiles -Path "$d"
    foreach ( $f in  $FilePaths ) {
      if ( $Verbose.IsPresent ) { Write-Output "Formatting file: ${f}" }
      Start-Process -FilePath "$ClangFormat" -ArgumentList "-i", "--style=file:${ConfigFile}", "--verbose", "$f" -NoNewWindow -Wait -ErrorAction Stop
    }
  }
  if ( $Verbose.IsPresent ) { Write-Output "Formatting completed successfully." }
  return $null
}

function Get-PowerShellFiles {
  [OutputType([String[]])]
  param(
    [Parameter(Mandatory, HelpMessage = 'Path to PowerShell script or directory for formatting.')]
    [String]
    $Path,
    [Parameter(HelpMessage = 'File patterns to include.')]
    [String[]]
    $Include = $PS_FILES_INCLUDE,
    [Parameter(HelpMessage = 'File patterns to exclude.')]
    [String[]]
    $Exclude = $PS_FILES_EXCLUDE
  )
  return (Get-ChildItem -Path "$Path" -File -Include $Include -Exclude $Exclude -Recurse | Select-Object -ExpandProperty FullName)
}

function Format-PowerShellFile {
  [OutputType()]
  param(
    [Parameter(Mandatory, HelpMessage = 'Path to PowerShell script for formatting.')]
    [String]
    $Path,
    [Parameter(HelpMessage = 'Text file encoding.')]
    [String]
    $Encoding = 'utf8',
    [Parameter(HelpMessage = 'Print the script analyzer rules.')]
    [Switch]
    $WriteRules
  )
  if ( $WriteRules.IsPresent ) { Get-ScriptAnalyzerRule }
  Install-ModuleIfMissing -ModuleName 'PSScriptAnalyzer'
  # Define custom PowerShell formatting rules. Implements rules similar to Prettier defaults.
  [Hashtable]$Settings = @{
    IncludeRules = @('PSPlaceOpenBrace', 'PSUseConsistentIndentation')
    Rules        = @{
      PSPlaceOpenBrace           = @{
        Enable     = $true
        OnSameLine = $true
      }
      PSUseConsistentIndentation = @{
        Enable              = $true
        IndentationSize     = 2
        PipelineIndentation = 'IncreaseIndentationForFirstPipeline'
        Kind                = 'space'
      }
    }
  }
  [String]$FileContents = (Get-Content -Path "$Path" -Encoding "$Encoding" | Out-String)
  [String]$Formatted = Invoke-Formatter -ScriptDefinition "$FileContents" -Settings $Settings
  Set-Content -Path "$Path" -Encoding "$Encoding" -Value "$Formatted" -Force
  return $null
}

function Format-PowerShellRecursive {
  [OutputType()]
  param(
    [Parameter(HelpMessage = 'Path to PowerShell script or directory for formatting.')]
    [String[]]
    $Paths = "$PS_DIR_PATHS",
    [Parameter(HelpMessage = 'Text file encoding.')]
    [String]
    $Encoding = 'utf8',
    [Parameter(HelpMessage = 'Print the script analyzer rules.')]
    [Switch]
    $WriteRules
  )
  if ( $Verbose.IsPresent ) { Write-Output "Formatting PowerShell files..." }
  if ( $WriteRules.IsPresent ) { Get-ScriptAnalyzerRule }
  Install-ModuleIfMissing -ModuleName "PSScriptAnalyzer"
  foreach ( $d in $Paths ) {
    [String[]]$FilePaths = Get-PowerShellFiles -Path "$d"
    foreach ( $f in $FilePaths ) {
      if ( $Verbose.IsPresent ) { Write-Output "Formatting file: ${f}" }
      Format-PowerShellFile -Path "$f" -Encoding "$Encoding"
    }
  }
  if ( $Verbose.IsPresent ) { Write-Output 'PowerShell script formatting complete.' }
  return $null
}

###
### Exports
###

Export-ModuleMember -Function Get-CppFiles, Format-CppRecursive
Export-ModuleMember -Function Install-ModuleIfMissing
Export-ModuleMember -Function Get-PowerShellFiles, Format-PowerShellFile, Format-PowerShellRecursive
