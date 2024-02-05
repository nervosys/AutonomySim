<#
FILENAME:
  build_docs.psm1
DESCRIPTION:
  PowerShell script to build documentation.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  11-17-2023
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).
  Script is intended to run from AutonomySim base project directory.

  Copyright © 2023 Nervosys, LLC
#>

###
### Variables
###

$PROJECT_DIR = "$PWD"
$BUILD_DIR   = "$PWD\temp\docs_build"

###
### Functions
###

function Build-Documentation {
    # Create and copy docs into build directory
    [System.IO.Directory]::CreateDirectory($BUILD_DIR)
    Copy-Item -Path "docs" -Destination "$BUILD_DIR\docs_root\docs" -Container -Force -Recurse  # -Exclude @("temp")
    Copy-Item -Path ".\*.md" -Destination "$BUILD_DIR\docs_root" -Force
    Move-Item -Path "$BUILD_DIR\docs_root\docs\mkdocs.yml" -Destination "$BUILD_DIR" -Force

    Write-Output '-----------------------------------------------------------------------------------------'
    Write-Output ' Building and serving documentation'
    Write-Output '-----------------------------------------------------------------------------------------'

    Set-Location $BUILD_DIR
    Start-Process -FilePath "mkdocs.exe" -ArgumentList "serve" -Wait -NoNewWindow
    Start-Process -FilePath "mkdocs.exe" -ArgumentList "build" -Wait -NoNewWindow
    Set-Location $PROJECT_DIR

    # Copy docs_root into docs_build
    Copy-Item -Path "$BUILD_DIR\docs_root\docs\images" -Destination "$BUILD_DIR\build\images" -Container -Force -Recurse
    Copy-Item -Path "$BUILD_DIR\docs_root\docs\misc"   -Destination "$BUILD_DIR\build\misc"   -Container -Force -Recurse
    Copy-Item -Path "$BUILD_DIR\docs_root\docs\paper"  -Destination "$BUILD_DIR\build\paper"  -Container -Force -Recurse

    Write-Output "Next Steps:"
    Write-Output " 1. git checkout gh-pages"
    Write-Output " 2. Copy $BUILD_DIR\build to root"
    Write-Output " 3. git push gh-pages"
}

###
### Exports
###

Export-ModuleMember -Function Build-Documentation
