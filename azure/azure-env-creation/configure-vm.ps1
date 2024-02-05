# configure-vm.ps1
#
# Configure Azure VM for AutonomySim

$AutonomySimInstallPath = "C:\AutonomySim\"
$AutonomySimBinaryZipUrl =  "https://github.com/nervosys/AutonomySim/releases/download/v1.3.1-windows/Blocks.zip"
$AutonomySimBinaryZipFilename = "Blocks.zip"
$AutonomySimBinaryPath = $AutonomySimInstallPath + "blocks\blocks\binaries\win64\blocks.exe"
$AutonomySimBinaryName = "Blocks"

$webClient = new-object System.Net.WebClient

# Install OpenSSH client
Add-WindowsCapability -Online -Name OpenSSH.Client~~~~0.0.1.0
# Install OpenSSH server
Add-WindowsCapability -Online -Name OpenSSH.Server~~~~0.0.1.0
# Enable SSH service/daemon
Start-Service sshd
Set-Service -Name sshd -StartupType 'Automatic'

# Install Chocolatey
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iwr https://chocolatey.org/install.ps1 -UseBasicParsing | iex
# Bypass confirmation checks
choco feature enable --name="'allowGlobalConfirmation'"
choco install python --version=3.8.2
choco install git
# C++ runtime
choco install vcredist-all
choco install directx

# Create folder and set as default directory
New-Item -ItemType directory -Path $AutonomySimInstallPath
cd $AutonomySimInstallPath

# Get and unzip AutonomySim
$webClient.DownloadFile($AutonomySimBinaryZipUrl, $AutonomySimInstallPath + $AutonomySimBinaryZipFilename)
Expand-Archive $AutonomySimBinaryZipFilename

# Firewall rule for AutonomySim
New-NetFirewallRule -DisplayName $AutonomySimBinaryName -Direction Inbound -Program $AutonomySimBinaryPath -Action Allow
