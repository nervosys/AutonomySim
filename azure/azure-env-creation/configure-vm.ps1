<#
FILENAME:
  configure-vm.ps1
DESCRIPTION:
  Configure Azure VM for AutonomySim.
AUTHOR:
  Adam Erickson (Nervosys)
DATE:
  2024-02-20
NOTES:
  Assumes: PowerShell version >= 7 and Visual Studio 2022 (version 17).
  
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

[String]$UNREAL_ENV_NAME = 'Blocks'
[String]$UNREAL_ENV_ZIP_FILENAME = "${UNREAL_ENV_NAME}.zip"
[String]$UNREAL_ENV_ZIP_URL =  "https://github.com/nervosys/AutonomySim/releases/download/v1.3.1-windows/${UNREAL_ENV_ZIP_FILENAME}"
[String]$UNREAL_ENV_BINARY_PATH = "${PROJECT_DIR}\blocks\blocks\binaries\win64\blocks.exe"

###
### Main
###

# Create WebClient object
[Net.WebClient]$WebClient = New-Object Net.WebClient

# Install OpenSSH client
Add-WindowsCapability -Online -Name 'OpenSSH.Client~~~~0.0.1.0'
# Install OpenSSH server
Add-WindowsCapability -Online -Name 'OpenSSH.Server~~~~0.0.1.0'
# Enable SSH service/daemon
Start-Service -Name 'sshd'
Set-Service -Name 'sshd' -StartupType 'Automatic'

# Install Chocolatey
Set-ExecutionPolicy Bypass -Scope Process -Force
# [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor 3072  # TLS v1.2 for .NET 4.0
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12  # TLS v1.2
Invoke-WebRequest 'https://chocolatey.org/install.ps1' -UseBasicParsing | Invoke-Expression

# Bypass confirmation checks
Start-Process -FilePath 'choco.exe' -ArgumentList @('feature', 'enable', '--name="allowGlobalConfirmation"') -NoNewWindow -Wait
Start-Process -FilePath 'choco.exe' -ArgumentList @('install', 'python', '--version=3.8.2') -NoNewWindow -Wait
Start-Process -FilePath 'choco.exe' -ArgumentList @('install', 'git') -NoNewWindow -Wait
Start-Process -FilePath 'choco.exe' -ArgumentList @('install', 'vcredist-all') -NoNewWindow -Wait
Start-Process -FilePath 'choco.exe' -ArgumentList @('install', 'directx') -NoNewWindow -Wait

# Create folder and set as default directory
New-Item -ItemType Directory -Path "$PROJECT_DIR"
Set-Location "$PROJECT_DIR"

# Get and unzip AutonomySim
$WebClient.DownloadFile("$UNREAL_ENV_ZIP_URL", "$PROJECT_DIR" + "$UNREAL_ENV_ZIP_FILENAME")
Expand-Archive "$UNREAL_ENV_ZIP_FILENAME"

# Firewall rule for AutonomySim
New-NetFirewallRule -DisplayName "$UNREAL_ENV_NAME" -Direction Inbound -Program "$UNREAL_ENV_BINARY_PATH" -Action Allow
