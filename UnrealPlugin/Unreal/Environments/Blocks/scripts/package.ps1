<#
FILENAME:
  package.ps1
DESCRIPTION:
  PowerShell script to...
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  11-17-2023
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).
  Script is meant to run from AutonomySim\Unreal\Environments\<Name> directory.
  For each `.uproject` file, run:
  - C:\Program Files\Epic Games\UE_5.3\Engine\Build\BatchFiles\Build.bat
  - C:\Program Files\Epic Games\UE_5.3\Engine\Build\BatchFiles\RunUAT.bat
USAGE:
  `powershell scripts\package.ps1`

  Copyright © 2023 Nervosys, LLC
#>

# Command-line arguments
param(
    [Parameter(Mandatory=$true, HelpMessage='Path to directory for package outputs.')]
    [String]
    $PackageOutputsDir,
    [Parameter(HelpMessage='Path to directory containing Unreal Engine build tools.')]
    [String]
    $UnrealBuildToolsDir = "${env:ProgramFiles}\Epic Games\UE_${UNREAL_VERSION}\Engine\Build\BatchFiles",
    [Parameter(HelpMessage="Unreal Engine version in format 'MAJOR.MINOR'.")]
    [String]
    $UnrealVersion = '5.3'
)

###
### Variables
###

# Static variables
$WORKING_DIR = "$PWD"

# Command-line arguments
$PACKAGE_OUTPUTS_DIR   = $PackageOutputsDir
$UNREAL_BUILDTOOLS_DIR = $UnrealBuildToolsDir
$UNREAL_VERSION        = $UnrealVersion

# Dynamic variables

###
### Main
###

Write-Output ''
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output ' Parameters'
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output " Unreal build tools directory: $UNREAL_BUILDTOOLS_DIR"
Write-Output " Package outputs directory:    $PACKAGE_OUTPUTS_DIR"
Write-Output '-----------------------------------------------------------------------------------------'
Write-Output ''

if ( -not (Test-Path -LiteralPath $UNREAL_BUILDTOOLS_DIR) ) {
    Write-Error "Unreal Engine batch files path not found: $UNREAL_BUILDTOOLS_DIR" -ErrorAction Stop
}

if ( -not (Test-Path -LiteralPath $PACKAGE_OUTPUTS_DIR) ) {
    Write-Error "Package output directory path not found: $PACKAGE_OUTPUTS_DIR" -ErrorAction Stop
}

$ProjectFiles = Get-ChildItem -Path *.uproject -File | Select-Object FullName
foreach ($ProjectFile in $ProjectFiles) {
        $ProjectName = [System.IO.Path]::GetFileNameWithoutExtension($ProjectFile.FullName)
        Write-Output "Packaging: $ProjectName"

        Start-Process -FilePath "$UNREAL_BUILDTOOLS_DIR\Build.bat" -ArgumentList "${ProjectName}Editor",'Win64','Development','-WarningsAsErrors',"$WORKING_DIR\$ProjectFile" -Wait -NoNewWindow
        if (!$?) { exit $LASTEXITCODE }  # exit on error
        # "%ToolPath%\RunUAT" -ScriptsForProject="%cd%\%%f" BuildCookRun -installed -nop4 -project="%cd%\%%f" -cook -stage -archive -archivedirectory="%OutPath%" -package -clientconfig=Development -ue4exe=UE4Editor-Cmd.exe -compressed -pak -prereqs -nodebuginfo -targetplatform=Win64 -build -utf8output -nocompile -nocompileeditor 
		# "%ToolPath%\RunUAT" BuildCookRun -project="%cd%\%%f" -noP4 -platform=Win64 -clientconfig=Development -serverconfig=Development -cook -rocket -allmaps -build -stage -NoCompile -nocompileeditor -pak -archive -archivedirectory="%OutPath%"

        Remove-Item -Path "$PACKAGE_OUTPUTS_DIR\$ProjectName" -Recurse -Force
        Start-Process -FilePath "$UNREAL_BUILDTOOLS_DIR\RunUAT.bat" -ArgumentList 'BuildCookRun',"-project=$WORKING_DIR\$ProjectFile",'-noP4','-platform=Win64','-clientconfig=Development','-cook','-build','-stage','-pak','-archive',"-archivedirectory=$PACKAGE_OUTPUTS_DIR",'-utf8output','-compressed','-prereqs' -Wait -NoNewWindow
		if (!$?) { exit $LASTEXITCODE }  # exit on error

        Move-Item -Path "$PACKAGE_OUTPUTS_DIR\WindowsNoEditor" -Destination "$PACKAGE_OUTPUTS_DIR\$ProjectName" -Force
		"start $ProjectName -windowed" | Out-File -Encoding 'ascii' -FilePath "$PACKAGE_OUTPUTS_DIR\$ProjectName\run.cmd"
        Write-Output "Unreal project packaging complete: $ProjectName"
}

exit 0
