<#
FILENAME:
  powershell_formatter.psm1
DESCRIPTION:
  PowerShell module for formatting scripts or directories.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-20
NOTES:
  Assumes: PowerShell version >= 7.
  
  Copyright © 2024 Nervosys, LLC
#>

# Install PowerShell module if not already installed.
Install-ModuleIfMissing {
  [OutputType()]
  param(
    [Parameter(Mandatory, HelpMessage = 'Name of the PowerShell module.')]
    [String]
    $ModuleName,
    [Parameter(HelpMessage = 'Switch to enable verbose mode.')]
    [Switch]
    $Silent
  )
  if ( -not $Silent.IsPresent ) { Write-Output 'Checking if moduled is installed...' }
  if ( -not (Get-Module -Name "$ModuleName" -ListAvailable | Select Name, Version) ) {
    if ( -not $Silent.IsPresent ) {
      Write-Output "Module not found: ${ModuleName}"
      Write-Output "Installing module: ${ModuleName}"
    }
    Install-Module -Name "$ModuleName" -Force -Confirm:$false
  }
  else {
    if ( -not $Silent.IsPresent ) { Write-Output "Module found: ${ModuleName}" }
  }
}

# Create array of PowerShell file (.ps1 .psm1) absolute paths.
Format-PowerShell {
  [OutputType()]
  param(
    [Parameter(Mandatory, HelpMessage = 'Path to PowerShell script or directory for formatting.')]
    [String]
    $Path,
    [Parameter(HelpMessage = 'Text file encoding.')]
    [String]
    $Encoding = 'utf8',
    [Parameter(HelpMessage = 'Switch for recursively formatting files in directory.')]
    [Switch]
    $Recurse
  )
  Install-ModuleIfMissing -ModuleName "PSScriptAnalyzer"
  # Get a list of file paths.
  [String[]]$FilePaths = if ( $Recurse.IsPresent ) {
        (Get-ChildItem -Path $Path -File -Include ('*.ps1', '*.psm1') -Recurse | Select FullName)
  }
  else { $Path }
  # Iterate over files, load to string, format string, save to file.
  foreach ( $f in $FilePaths ) {
    [String]$FileContents = (Get-Content -Encoding "$Encoding" -Path "$f" | Out-String)
    Invoke-Formatter -ScriptDefinition $FileContents
    Set-Content -Path "$f" -Encoding "$Encoding" -Value "$FileContents"
  }
  Write-Output 'PowerShell script formatting complete.'
}
