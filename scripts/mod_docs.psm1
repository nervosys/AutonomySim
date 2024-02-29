<#
FILENAME:
  mod_docs.psm1
DESCRIPTION:
  PowerShell script to build documentation.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-20
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).
  Script is intended to run from AutonomySim base project directory.

  Copyright © 2024 Nervosys, LLC
#>

[String]$PROJECT_DIR = (Split-Path -Parent -Path (Split-Path -Parent -Path "$PSScriptRoot"))
[String]$SCRIPT_DIR = (Split-Path -Parent -Path "$PSScriptRoot")

# Utilities
# imports: Add-Directories, Remove-ItemSilent, Remove-TempDirectories, Invoke-Fail,
#   Test-WorkingDirectory, Test-VariableDefined, Get-EnvVariables, Get-ProgramVersion,
#   Get-VersionMajorMinor, Get-VersionMajorMinorBuild, Get-WindowsInfo, Get-WindowsVersion,
#   Get-Architecture, Get-ArchitectureWidth, Set-ProcessorCount
Import-Module "${SCRIPT_DIR}\mod_utils.psm1"

###
### Functions
###

function Build-Documentation {
  [OutputType()]
  param(
    [Parameter()]
    [String]
    $ProjectDir = "$PROJECT_DIR",
    [Parameter(HelpMessage = 'Path to `make.exe` program. Set to Windows 10/11 default path.')]
    [String]
    $MakeExe = 'C:\Program Files (x86)\GnuWin32\bin\make.exe',
    [Parameter()]
    [Boolean]
    $ServeDocs = $false
  )
  if ( -not (Test-Program -Program "mkdocs.exe") ) {
    Invoke-Fail -ErrorMessage 'Error: Program not found: mkdocs.exe'
  }
  if ( -not (Test-Program -Program "$MakeExe") ) {
    Invoke-Fail -ErrorMessage 'Error: Program not found: make.exe'
  }

  # Ensure Python environment is properly configured.
  # python3 -m venv .env  # virtual environment to avoid dependecy issues
  # source ./.env/bin/activate
  # python3 -m pip install --upgrade pip
  # pip install mkdocs mkdocs-material pymdown-extensions  # mkdocstrings[python]
  # pip install sphinx sphinx-immaterial numpy msgpack-rpc-python
  # pip install breathe exhale  # for doxygen -> sphinx -> html conversion

  if ( $Verbose.IsPresent ) {
    Write-Output '-------------------------------------------------------------------------------'
    Write-Output ' Building documentation...'
    Write-Output '-------------------------------------------------------------------------------'
  }
  
  # Generate C++ API documentation.
  Set-Location "${PROJECT_DIR}\AutonomyLib\docs"
  Start-Process -FilePath "$MakeExe" -ArgumentList 'html' -Wait -NoNewWindow
  Copy-Item -Path "${PROJECT_DIR}\AutonomyLib\docs\_build" -Filter '*' -Destination "${PROJECT_DIR}\docs\api\cpp" -Recurse -Force
  
  # Generate Python API documentation.
  Set-Location "${PROJECT_DIR}\python\docs"
  Start-Process -FilePath "$MakeExe" -ArgumentList 'html' -Wait -NoNewWindow
  Copy-Item -Path "${PROJECT_DIR}\python\docs\_build" -Filter '*' -Destination "${PROJECT_DIR}\docs\api\python" -Recurse -Force

  # Build main project documentation.
  Set-Location "${PROJECT_DIR}"
  Copy-Item -Path "${ProjectDir}\README.md" -Destination "${ProjectDir}\docs" -Force
  Start-Process -FilePath 'mkdocs.exe' -ArgumentList 'build' -Wait -NoNewWindow

  if ( $ServeDocs ) {
    Start-Process -FilePath 'mkdocs.exe' -ArgumentList 'serve' -NoNewWind
  }

  if ( $Verbose.IsPresent ) {
    Write-Output 'Next Steps:'
    Write-Output '  1. git checkout gh-pages'
    Write-Output '  2. Copy $BuildDir\build to root'
    Write-Output '  3. git push gh-pages'
  }
  
  return $null
}

###
### Exports
###

Export-ModuleMember -Function Build-Documentation
