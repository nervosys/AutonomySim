# Quick Setup Script for AutonomySim + Unreal Engine 5.6

$ErrorActionPreference = "Stop"

Write-Host "╔══════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║     AutonomySim + Unreal Engine 5.6 Setup Script          ║" -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Configuration
$UE5Path = "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
$ProjectsDir = "C:\Users\adamm\UE5Projects"
$ProjectName = "AutonomySimDemo"
$ProjectPath = "$ProjectsDir\$ProjectName"
$PluginSource = "C:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim\UnrealPlugin\Unreal\Plugins\AutonomySim"
$RustBackendPath = "C:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim\rust"

# Step 1: Verify UE5 Installation
Write-Host "[1/6] Checking Unreal Engine 5.6 installation..." -ForegroundColor Yellow
if (Test-Path $UE5Path) {
    Write-Host "  ✅ Unreal Engine 5.6 found!" -ForegroundColor Green
}
else {
    Write-Host "  ❌ Unreal Engine 5.6 not found at: $UE5Path" -ForegroundColor Red
    Write-Host "  Please install UE5.6 from Epic Games Launcher" -ForegroundColor Red
    exit 1
}

# Step 2: Create Project Directory
Write-Host ""
Write-Host "[2/6] Setting up project directory..." -ForegroundColor Yellow
if (!(Test-Path $ProjectsDir)) {
    New-Item -ItemType Directory -Force -Path $ProjectsDir | Out-Null
    Write-Host "  ✅ Created $ProjectsDir" -ForegroundColor Green
}
else {
    Write-Host "  ✅ Project directory exists" -ForegroundColor Green
}

# Step 3: Check if project exists
Write-Host ""
Write-Host "[3/6] Checking for existing project..." -ForegroundColor Yellow
$ProjectFile = "$ProjectPath\$ProjectName.uproject"

if (Test-Path $ProjectFile) {
    Write-Host "  ℹ️  Project already exists at: $ProjectPath" -ForegroundColor Yellow
    $response = Read-Host "  Do you want to use existing project? (Y/N)"
    if ($response -ne "Y" -and $response -ne "y") {
        Write-Host "  Please manually delete the project or choose a different name" -ForegroundColor Red
        exit 1
    }
}
else {
    Write-Host "  ℹ️  Project doesn't exist yet" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "  Next steps to create the project:" -ForegroundColor Cyan
    Write-Host "    1. Open Epic Games Launcher" -ForegroundColor White
    Write-Host "    2. Launch Unreal Engine 5.6" -ForegroundColor White
    Write-Host "    3. Create New Project:" -ForegroundColor White
    Write-Host "       - Template: Blank (C++)" -ForegroundColor White
    Write-Host "       - Name: $ProjectName" -ForegroundColor White
    Write-Host "       - Location: $ProjectsDir" -ForegroundColor White
    Write-Host "    4. Close the project after creation" -ForegroundColor White
    Write-Host ""
    $response = Read-Host "  Press Enter when project is created (or 'N' to exit)"
    if ($response -eq "N" -or $response -eq "n") {
        exit 0
    }
    
    if (!(Test-Path $ProjectFile)) {
        Write-Host "  ❌ Project file not found. Please create the project first." -ForegroundColor Red
        exit 1
    }
}

# Step 4: Install AutonomySim Plugin
Write-Host ""
Write-Host "[4/6] Installing AutonomySim plugin..." -ForegroundColor Yellow
$PluginDest = "$ProjectPath\Plugins\AutonomySim"

if (!(Test-Path "$ProjectPath\Plugins")) {
    New-Item -ItemType Directory -Force -Path "$ProjectPath\Plugins" | Out-Null
}

if (Test-Path $PluginDest) {
    Write-Host "  ℹ️  Plugin already installed, updating..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $PluginDest
}

Write-Host "  Copying plugin files..." -ForegroundColor Gray
Copy-Item -Recurse $PluginSource $PluginDest
Write-Host "  ✅ AutonomySim plugin installed!" -ForegroundColor Green

# Step 5: Generate Visual Studio Files
Write-Host ""
Write-Host "[5/6] Generating Visual Studio project files..." -ForegroundColor Yellow
Write-Host "  This may take a minute..." -ForegroundColor Gray

$GenerateFiles = "$UE5Path".Replace("UnrealEditor.exe", "..\..\..\..\Build\BatchFiles\GenerateProjectFiles.bat")
if (Test-Path $GenerateFiles) {
    & $GenerateFiles -project="$ProjectFile" -game -engine 2>&1 | Out-Null
    Write-Host "  ✅ Project files generated!" -ForegroundColor Green
}
else {
    Write-Host "  ⚠️  Could not auto-generate. Right-click .uproject → Generate VS files" -ForegroundColor Yellow
}

# Step 6: Instructions for Building
Write-Host ""
Write-Host "[6/6] Final setup steps..." -ForegroundColor Yellow
Write-Host ""
Write-Host "╔══════════════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║                SETUP READY - Next Steps                     ║" -ForegroundColor Green
Write-Host "╚══════════════════════════════════════════════════════════════╝" -ForegroundColor Green
Write-Host ""
Write-Host "1. Build the project:" -ForegroundColor Cyan
Write-Host "   - Open: $ProjectPath\$ProjectName.sln" -ForegroundColor White
Write-Host "   - Set configuration: Development Editor" -ForegroundColor White
Write-Host "   - Build Solution (Ctrl+Shift+B)" -ForegroundColor White
Write-Host "   - Wait 5-10 minutes for first build" -ForegroundColor White
Write-Host ""
Write-Host "2. Launch Unreal Editor:" -ForegroundColor Cyan
Write-Host "   - Open: $ProjectFile" -ForegroundColor White
Write-Host "   - Enable AutonomySim plugin when prompted" -ForegroundColor White
Write-Host "   - Create a new level or open existing" -ForegroundColor White
Write-Host ""
Write-Host "3. Configure RPC Server:" -ForegroundColor Cyan
Write-Host "   - In Level Blueprint (Blueprints → Open Level Blueprint)" -ForegroundColor White
Write-Host "   - Add: Event BeginPlay → Enable AutonomySim RPC (Port: 41451)" -ForegroundColor White
Write-Host ""
Write-Host "4. Press PLAY in UE5 Editor" -ForegroundColor Cyan
Write-Host ""
Write-Host "5. Run Rust backend:" -ForegroundColor Cyan
Write-Host "   cd $RustBackendPath" -ForegroundColor White
Write-Host "   cargo run --example unreal_robotic_swarm --features unreal --release" -ForegroundColor White
Write-Host ""
Write-Host "══════════════════════════════════════════════════════════════" -ForegroundColor Green
Write-Host ""

# Offer to open Visual Studio
$response = Read-Host "Do you want to open Visual Studio now to build? (Y/N)"
if ($response -eq "Y" -or $response -eq "y") {
    $SlnFile = "$ProjectPath\$ProjectName.sln"
    if (Test-Path $SlnFile) {
        Write-Host "Opening Visual Studio..." -ForegroundColor Green
        Start-Process $SlnFile
    }
    else {
        Write-Host "Solution file not found: $SlnFile" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Setup script complete! Follow the steps above to continue." -ForegroundColor Green
Write-Host "See UNREAL_ENGINE_56_SETUP.md for detailed instructions." -ForegroundColor Gray
