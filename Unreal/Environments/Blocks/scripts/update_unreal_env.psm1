<#
FILENAME:
  update_unreal_env.psm1
DESCRIPTION:
  PowerShell script to update Unreal Engine environment.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  11-17-2023
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).
  Script is intended to run from `AutonomySim\Unreal\Environments\<Environment>`
  directory, which we invoke from `AutonomySim\scripts\build.ps1`.
USAGE:
  `powershell AutonomySim\scripts\update_unreal_env.psm1`

  Copyright © 2023 Nervosys, LLC
#>

###
### Variables
###

$AUTONOMYSIM_PATH = '..\..\..'

###
### Functions
###

function Test-DirectoryPath {
    param(
        [Parameter()]
        [String]
        $Path = $AUTONOMYSIM_PATH
    )
    Write-Output "Testing Path: $Path"
    if ( -not (Test-Path -LiteralPath $Path) ) {
    	Write-Error "Error: Path not found." -ErrorAction Stop
    }
}

function Copy-UnrealEnvItems {
    param(
        [Parameter()]
        [String]
        $Path = $AUTONOMYSIM_PATH
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
    Copy-Item -Path "$Path\Unreal\Plugins\AutonomySim\*" -Destination 'Plugins\AutonomySim' -Container -Force -Recurse -Exclude @("temp")
    Copy-Item -Path "$Path\AutonomyLib" -Destination 'Plugins\AutonomySim\Source\AutonomyLib' -Container -Force -Recurse -Exclude @("temp")
    Copy-Item -Path "$Path\Unreal\Environments\Blocks\*.cmd" -Destination '.' -Container -Force -Recurse
    Copy-Item -Path "$Path\Unreal\Environments\Blocks\*.sh" -Destination '.' -Container -Force -Recurse
}

function Remove-UnrealEnvItems {
    Remove-Item -Path 'Build' -Recurse
    Remove-Item -Path 'Binaries' -Recurse
    Remove-Item -Path 'Intermediate' -Recurse
    Remove-Item -Path 'Saved' -Recurse
    [System.IO.Directory]::CreateDirectory('Saved\logs')
    Remove-Item -Path 'Plugins\AutonomySim\Binaries' -Recurse
    Remove-Item -Path 'Plugins\AutonomySim\Intermediate' -Recurse
    Remove-Item -Path 'Plugins\AutonomySim\Saved' -Recurse
    Remove-Item -Path *.sln
}

function Invoke-VsUnrealProjectFileGenerator {
    # Visual Studio Unreal Engine Project File Generator
    #Remove-Item -Path 'gen_temp.txt'
    # Get absolute path to Unreal Version Selector from `rungenproj` registry entry and save to single-line text file.
    # The `rungenproj` program, `UnrealVersionSelector.exe`, generates Visual Studio project files.
    # NOTE: Set Out-File (>) encoding to ASCII for consistency with older Set-Content command
    $GeneratorRegistryKey = 'Registry::HKEY_CLASSES_ROOT\Unreal.ProjectFile\shell\rungenproj'
    # Default on Windows 10/11: "C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe"
    $GeneratorPath = (Get-ItemProperty $GeneratorRegistryKey -Name 'Icon').Icon
    #(Get-ItemProperty 'Registry::HKEY_CLASSES_ROOT\Unreal.ProjectFile\shell\rungenproj' -Name 'Icon').Icon |
    #    Out-File -Encoding 'ascii' -FilePath 'gen_temp.txt'
    #$gen_bin = Get-Content 'gen_temp.txt' -Encoding 'ascii' -ReadCount 1  # set variable to `UnrealVersionSelector.exe` path string
    #Remove-Item -Path 'gen_temp.txt'  # remove temporary file
    $ProjectFiles = Get-ChildItem -Path *.uproject -File | Select-Object FullName
    foreach ($ProjectFile in $ProjectFiles) {
      $ProjectName = [System.IO.Path]::GetFileNameWithoutExtension($ProjectFile.FullName)
    	Write-Output "Generating Visual Studio project files: $ProjectName"    	
      Start-Process -FilePath $GeneratorPath -ArgumentList "/projectfiles","$ProjectFile" -Wait -NoNewWindow  # $gen_bin /projectfiles "$PWD\$Project"
    }
}

###
### Exports
###

Export-ModuleMember -Function Test-DirectoryPath
Export-ModuleMember -Function Copy-UnrealEnvItems
Export-ModuleMember -Function Remove-UnrealEnvItems
Export-ModuleMember -Function Invoke-VsUnrealProjectFileGenerator
