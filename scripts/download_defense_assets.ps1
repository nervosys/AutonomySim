# AutonomySim Defense Robotics Assets Download Script
# Downloads high-fidelity environments and UGV/UAV/USV/UUV assets
# Includes popular military drones used in Ukraine and other conflicts

param(
    [string]$DownloadPath = ".\UnrealAssets\DefenseRobotics",
    [switch]$SkipEnvironments = $false,
    [switch]$SkipVehicles = $false,
    [switch]$SkipWeapons = $false,
    [switch]$DryRun = $false
)

$ErrorActionPreference = "Continue"

# Create download directory
$DownloadPath = Resolve-Path $DownloadPath -ErrorAction SilentlyContinue
if (-not $DownloadPath) {
    $DownloadPath = Join-Path (Get-Location) "UnrealAssets\DefenseRobotics"
    New-Item -ItemType Directory -Path $DownloadPath -Force | Out-Null
}

Write-Host "╔═══════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║   AUTONOMYSIM - DEFENSE ROBOTICS ASSETS DOWNLOADER        ║" -ForegroundColor Cyan
Write-Host "╚═══════════════════════════════════════════════════════════════╝`n" -ForegroundColor Cyan

Write-Host "Download Location: $DownloadPath`n" -ForegroundColor Yellow

# Helper function to download files
function Download-Asset {
    param(
        [string]$Url,
        [string]$FileName,
        [string]$Category,
        [string]$Description
    )
    
    $FilePath = Join-Path $DownloadPath "$Category\$FileName"
    $FileDir = Split-Path $FilePath -Parent
    
    if (-not (Test-Path $FileDir)) {
        New-Item -ItemType Directory -Path $FileDir -Force | Out-Null
    }
    
    if (Test-Path $FilePath) {
        Write-Host "  ✓ Already exists: $FileName" -ForegroundColor Green
        return $true
    }
    
    Write-Host "  ⬇ Downloading: $Description" -ForegroundColor Cyan
    Write-Host "    URL: $Url" -ForegroundColor Gray
    
    if ($DryRun) {
        Write-Host "    [DRY RUN] Would download to: $FilePath" -ForegroundColor Yellow
        return $true
    }
    
    try {
        $ProgressPreference = 'SilentlyContinue'
        Invoke-WebRequest -Uri $Url -OutFile $FilePath -UseBasicParsing
        Write-Host "  ✓ Downloaded: $FileName" -ForegroundColor Green
        return $true
    }
    catch {
        Write-Host "  ✗ Failed: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# ═══════════════════════════════════════════════════════════════
# 1. HIGH-FIDELITY ENVIRONMENTS
# ═══════════════════════════════════════════════════════════════

if (-not $SkipEnvironments) {
    Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
    Write-Host "1. HIGH-FIDELITY REAL-WORLD ENVIRONMENTS" -ForegroundColor Cyan
    Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White
    
    # Unreal Engine Marketplace (Free) Environments
    $environments = @(
        @{
            Name        = "Military Base Pack"
            Url         = "https://github.com/EpicGames/UnrealEngine/releases/download/5.3.2-release/MilitaryBase.zip"
            File        = "MilitaryBase.zip"
            Description = "Military airbase with runways, hangars, control tower"
        },
        @{
            Name        = "Urban Warfare Pack"
            Url         = "https://github.com/unrealengine/urbanwarfare/archive/refs/heads/main.zip"
            File        = "UrbanWarfare.zip"
            Description = "Eastern European urban environment with damaged buildings"
        },
        @{
            Name        = "Desert Military Outpost"
            Url         = "https://github.com/ue4marketplace/DesertOutpost/archive/main.zip"
            File        = "DesertOutpost.zip"
            Description = "Middle East desert military installation"
        },
        @{
            Name        = "Forest Terrain Pack"
            Url         = "https://github.com/ue5/ForestTerrain/archive/refs/heads/master.zip"
            File        = "ForestTerrain.zip"
            Description = "Dense forest environment for drone operations"
        },
        @{
            Name        = "Coastal Naval Base"
            Url         = "https://github.com/unrealengine/NavalBase/archive/main.zip"
            File        = "CoastalNavalBase.zip"
            Description = "Port facility with docks for USV/UUV testing"
        }
    )
    
    Write-Host "Note: Unreal Marketplace assets require manual download" -ForegroundColor Yellow
    Write-Host "Creating download list for manual acquisition...`n" -ForegroundColor Yellow
    
    $manualDownloads = @"
UNREAL MARKETPLACE ENVIRONMENTS (Require Epic Games Account)
═══════════════════════════════════════════════════════════════

HIGH-QUALITY REAL-WORLD ENVIRONMENTS:

1. MILITARY BASES
   • Military Base Pack - https://www.unrealengine.com/marketplace/military-base
   • Tactical Operations Base - https://www.unrealengine.com/marketplace/tob
   • Forward Operating Base - https://www.unrealengine.com/marketplace/fob
   
2. URBAN ENVIRONMENTS
   • War-Torn City Pack - https://www.unrealengine.com/marketplace/war-city
   • Eastern European Town - https://www.unrealengine.com/marketplace/ee-town
   • Post-Soviet Industrial - https://www.unrealengine.com/marketplace/soviet-industrial
   
3. NATURAL TERRAIN
   • Realistic Landscapes Vol. 1-3 - https://www.unrealengine.com/marketplace/landscapes
   • Forest Terrain Pack - https://www.unrealengine.com/marketplace/forest-terrain
   • Desert Military Zone - https://www.unrealengine.com/marketplace/desert-zone
   
4. COASTAL/MARITIME
   • Naval Port Facility - https://www.unrealengine.com/marketplace/naval-port
   • Coastal Defense System - https://www.unrealengine.com/marketplace/coastal-defense
   • Ocean/Underwater Pack - https://www.unrealengine.com/marketplace/ocean-underwater

ALTERNATIVE FREE RESOURCES:

1. Quixel Megascans (Free with Epic account)
   https://quixel.com/megascans
   - Photogrammetry environments
   - Military structures and props
   - Natural terrain scans
   
2. Cesium for Unreal (Real-world terrain)
   https://cesium.com/platform/cesium-for-unreal/
   - Real-world 3D terrain from Google/Bing
   - Photorealistic satellite imagery
   - Elevation data
   
3. OpenStreetMap to Unreal
   https://github.com/ue4plugins/StreetMap
   - Convert OSM data to 3D cities
   - Real locations worldwide

"@
    
    $manualDownloads | Out-File -FilePath (Join-Path $DownloadPath "MANUAL_DOWNLOADS_ENVIRONMENTS.txt") -Encoding UTF8
    Write-Host "✓ Created manual download list: MANUAL_DOWNLOADS_ENVIRONMENTS.txt`n" -ForegroundColor Green
}

# ═══════════════════════════════════════════════════════════════
# 2. UAV (UNMANNED AERIAL VEHICLES) - UKRAINE WAR DRONES
# ═══════════════════════════════════════════════════════════════

if (-not $SkipVehicles) {
    Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
    Write-Host "2. UAV ASSETS - UKRAINE CONFLICT DRONES" -ForegroundColor Cyan
    Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White
    
    $uavAssets = @"
UAV 3D MODELS - UKRAINE WAR & MODERN MILITARY DRONES
═══════════════════════════════════════════════════════════════

WIDELY USED IN UKRAINE:

1. DJI MAVIC 3 (Reconnaissance & Artillery Spotting)
   • Sketchfab: https://sketchfab.com/3d-models/dji-mavic-3
   • TurboSquid: https://www.turbosquid.com/3d-models/dji-mavic-3
   • CGTrader: https://www.cgtrader.com/3d-models/vehicle/other/dji-mavic-3
   
2. DJI PHANTOM 4 PRO (Modified for Munitions Drop)
   • Free3D: https://free3d.com/3d-model/dji-phantom-4-pro
   • GrabCAD: https://grabcad.com/library/dji-phantom-4-pro
   • Sketchfab: https://sketchfab.com/3d-models/phantom-4-pro
   
3. AUTEL EVO II (Reconnaissance)
   • CGTrader: https://www.cgtrader.com/3d-models/vehicle/other/autel-evo-ii
   • TurboSquid: https://www.turbosquid.com/3d-models/autel-evo-ii
   
4. TB2 BAYRAKTAR (Turkish Attack Drone - High-Profile in Ukraine)
   • GrabCAD: https://grabcad.com/library/bayraktar-tb2
   • Sketchfab: https://sketchfab.com/3d-models/bayraktar-tb2
   • Free Download: https://www.cgtrader.com/free-3d-models/aircraft/other/bayraktar-tb2
   
5. SWITCHBLADE 300/600 (Loitering Munition)
   • Sketchfab: https://sketchfab.com/3d-models/switchblade-300
   • CGTrader: https://www.cgtrader.com/3d-models/aircraft/military/switchblade-uav
   
6. RECONNAISSANCE QUADCOPTERS (Modified Commercial)
   • Generic Quadcopter Pack: https://sketchfab.com/3d-models/quadcopter-drone
   • Military Drone Pack: https://www.cgtrader.com/3d-models/aircraft/military/military-drones-pack

WESTERN/NATO DRONES:

7. MQ-9 REAPER (US Predator)
   • Free3D: https://free3d.com/3d-model/mq-9-reaper
   • GrabCAD: https://grabcad.com/library/mq-9-reaper
   • Sketchfab: https://sketchfab.com/3d-models/mq-9-reaper
   
8. RQ-4 GLOBAL HAWK
   • GrabCAD: https://grabcad.com/library/rq-4-global-hawk
   • Free3D: https://free3d.com/3d-model/global-hawk
   
9. BLACK HORNET NANO (Infantry Recon)
   • Sketchfab: https://sketchfab.com/3d-models/black-hornet-nano
   • CGTrader: https://www.cgtrader.com/3d-models/aircraft/military/black-hornet

RUSSIAN DRONES:

10. ORLAN-10 (Russian Recon - Frequently Shot Down)
    • GrabCAD: https://grabcad.com/library/orlan-10
    • Sketchfab: https://sketchfab.com/3d-models/orlan-10
    
11. LANCET (Russian Loitering Munition)
    • CGTrader: https://www.cgtrader.com/3d-models/aircraft/military/lancet-uav
    
12. SHAHED-136 (Iranian Kamikaze Drone Used by Russia)
    • Sketchfab: https://sketchfab.com/3d-models/shahed-136
    • Free Models: https://www.cgtrader.com/free-3d-models/aircraft/military/shahed-136

FPV RACING DRONES (Modified for Military Use):

13. FPV RACING DRONE PACK
    • Sketchfab: https://sketchfab.com/3d-models/fpv-racing-drone
    • TurboSquid: https://www.turbosquid.com/3d-models/fpv-drone
    • CGTrader: https://www.cgtrader.com/3d-models/vehicle/other/fpv-racing-drone

DOWNLOAD SITES:

• Sketchfab (Many free): https://sketchfab.com/search?q=military+drone&type=models
• GrabCAD (CAD models): https://grabcad.com/library?q=military%20drone
• Free3D: https://free3d.com/3d-models/military-drone
• CGTrader: https://www.cgtrader.com/3d-models/aircraft/military
• NASA 3D Resources: https://nasa3d.arc.nasa.gov/models

"@
    
    $uavAssets | Out-File -FilePath (Join-Path $DownloadPath "MANUAL_DOWNLOADS_UAV.txt") -Encoding UTF8
    Write-Host "✓ Created UAV asset list: MANUAL_DOWNLOADS_UAV.txt`n" -ForegroundColor Green
}

# ═══════════════════════════════════════════════════════════════
# 3. UGV (UNMANNED GROUND VEHICLES)
# ═══════════════════════════════════════════════════════════════

if (-not $SkipVehicles) {
    Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
    Write-Host "3. UGV ASSETS - GROUND ROBOTS" -ForegroundColor Cyan
    Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White
    
    $ugvAssets = @"
UGV 3D MODELS - MILITARY GROUND ROBOTS
═══════════════════════════════════════════════════════════════

COMBAT & RECONNAISSANCE:

1. BOSTON DYNAMICS SPOT (Modified for Military)
   • Free: https://github.com/boston-dynamics/spot-sdk/tree/master/docs/concepts/robot_electrical
   • Sketchfab: https://sketchfab.com/3d-models/boston-dynamics-spot
   • GrabCAD: https://grabcad.com/library/boston-dynamics-spot
   
2. TALON ROBOT (EOD/IED Disposal)
   • GrabCAD: https://grabcad.com/library/foster-miller-talon
   • Free3D: https://free3d.com/3d-model/talon-robot
   
3. PACKBOT (US Military EOD)
   • GrabCAD: https://grabcad.com/library/irobot-packbot
   • Sketchfab: https://sketchfab.com/3d-models/packbot
   
4. MILREM THEMIS (Estonian Armed UGV)
   • Official: https://milrem.com/themis/
   • CGTrader: https://www.cgtrader.com/3d-models/vehicle/military/themis-ugv
   
5. RIPSAW M5 (Fast Attack UGV)
   • GrabCAD: https://grabcad.com/library/ripsaw-m5
   • Sketchfab: https://sketchfab.com/3d-models/ripsaw-ugv

UKRAINE CONFLICT UGVs:

6. GROUND DRONE (THeMIS Variations)
   • Multiple variants used for supply/attack
   • CGTrader: https://www.cgtrader.com/3d-models/vehicle/military/ground-combat-drone
   
7. TELETANK T-18 (Remote-Controlled Tank Chassis)
   • Historical/Modern variants
   • GrabCAD: https://grabcad.com/library/remote-tank

LOGISTICS & SUPPORT:

8. CLEARPATH HUSKY (Research Platform)
   • Official: https://clearpathrobotics.com/husky-unmanned-ground-vehicle-robot/
   • GrabCAD: https://grabcad.com/library/clearpath-husky
   
9. AUTONOMOUS MULE (Supply Transport)
   • Sketchfab: https://sketchfab.com/3d-models/military-mule-ugv
   • TurboSquid: https://www.turbosquid.com/3d-models/military-robot-mule

DOWNLOAD LOCATIONS:

• GrabCAD UGV Collection: https://grabcad.com/library?q=ugv
• Military Robots Pack: https://www.cgtrader.com/3d-model-collections/military-robots
• ROS Robot Models: https://github.com/osrf/gazebo_models

"@
    
    $ugvAssets | Out-File -FilePath (Join-Path $DownloadPath "MANUAL_DOWNLOADS_UGV.txt") -Encoding UTF8
    Write-Host "✓ Created UGV asset list: MANUAL_DOWNLOADS_UGV.txt`n" -ForegroundColor Green
}

# ═══════════════════════════════════════════════════════════════
# 4. USV (UNMANNED SURFACE VEHICLES) & UUV (UNDERWATER)
# ═══════════════════════════════════════════════════════════════

if (-not $SkipVehicles) {
    Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
    Write-Host "4. USV/UUV ASSETS - MARITIME DRONES" -ForegroundColor Cyan
    Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White
    
    $maritimeAssets = @"
USV/UUV 3D MODELS - MARITIME DRONES
═══════════════════════════════════════════════════════════════

UKRAINE MARITIME DRONES (Used Against Russian Fleet):

1. SEA BABY (Ukrainian Attack USV)
   • Recent design, limited 3D models available
   • Reference images for modeling
   
2. MAGURA V5 (Ukrainian Naval Drone)
   • Jet-ski based explosive drone
   • Similar models: https://sketchfab.com/3d-models/jet-ski

US/NATO USVs:

3. SEA HUNTER (DARPA Anti-Submarine USV)
   • GrabCAD: https://grabcad.com/library/sea-hunter-usv
   • Sketchfab: https://sketchfab.com/3d-models/usv-sea-hunter
   
4. MANTAS T-12 (Patrol USV)
   • CGTrader: https://www.cgtrader.com/3d-models/watercraft/other/mantas-usv
   
5. WAVE GLIDER (Ocean Research)
   • Free3D: https://free3d.com/3d-model/wave-glider

UNDERWATER VEHICLES (UUV):

6. BLUEFIN-21 (Submarine AUV)
   • GrabCAD: https://grabcad.com/library/bluefin-21-auv
   • Sketchfab: https://sketchfab.com/3d-models/bluefin-auv
   
7. REMUS 100/600 (Mine Countermeasures)
   • GrabCAD: https://grabcad.com/library/remus-100
   • Naval assets: https://www.cgtrader.com/3d-models/watercraft/military/remus-auv
   
8. TORPEDO SHAPE AUVs
   • Generic: https://sketchfab.com/3d-models/autonomous-underwater-vehicle
   • Research: https://grabcad.com/library/auv-autonomous-underwater-vehicle

GENERIC MARITIME DRONES:

9. PATROL BOAT USV
   • Sketchfab: https://sketchfab.com/3d-models/usv-patrol-boat
   • TurboSquid: https://www.turbosquid.com/3d-models/unmanned-surface-vehicle

DOWNLOAD SITES:

• Maritime Drones: https://www.cgtrader.com/3d-models/watercraft/military
• Naval Models: https://grabcad.com/library?q=usv
• Underwater: https://sketchfab.com/search?q=auv&type=models

"@
    
    $maritimeAssets | Out-File -FilePath (Join-Path $DownloadPath "MANUAL_DOWNLOADS_USV_UUV.txt") -Encoding UTF8
    Write-Host "✓ Created USV/UUV asset list: MANUAL_DOWNLOADS_USV_UUV.txt`n" -ForegroundColor Green
}

# ═══════════════════════════════════════════════════════════════
# 5. WEAPONS & MUNITIONS
# ═══════════════════════════════════════════════════════════════

if (-not $SkipWeapons) {
    Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
    Write-Host "5. WEAPONS & MUNITIONS ASSETS" -ForegroundColor Cyan
    Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White
    
    $weaponsAssets = @"
WEAPONS & MUNITIONS 3D MODELS
═══════════════════════════════════════════════════════════════

LOITERING MUNITIONS:

1. SWITCHBLADE (US)
   • https://sketchfab.com/3d-models/switchblade-300
   • https://www.cgtrader.com/3d-models/aircraft/military/switchblade-uav
   
2. HERO SERIES (Israel)
   • https://sketchfab.com/3d-models/hero-loitering-munition
   
3. PHOENIX GHOST (US, Supplied to Ukraine)
   • Limited models, use Switchblade as reference

MISSILES & ROCKETS:

4. JAVELIN (Anti-Tank)
   • GrabCAD: https://grabcad.com/library/javelin-missile
   • Free3D: https://free3d.com/3d-model/javelin-missile
   
5. HIMARS ROCKETS (M31 GMLRS)
   • GrabCAD: https://grabcad.com/library/m31-gmlrs
   • CGTrader: https://www.cgtrader.com/3d-models/military/weapon/himars-rocket
   
6. NLAW (Next-generation Light Anti-tank Weapon)
   • Sketchfab: https://sketchfab.com/3d-models/nlaw
   • GrabCAD: https://grabcad.com/library/nlaw

ARTILLERY MUNITIONS:

7. 155mm ARTILLERY SHELLS
   • GrabCAD: https://grabcad.com/library/155mm-artillery-shell
   • Various types: standard, guided, rocket-assisted
   
8. DRONE-DROPPED MUNITIONS
   • Modified grenades, mortar shells
   • Generic models: https://sketchfab.com/3d-models/military-munitions

GENERIC WEAPON SYSTEMS:

9. MILITARY WEAPONS PACK
   • Sketchfab: https://sketchfab.com/3d-models/military-weapons-collection
   • CGTrader: https://www.cgtrader.com/3d-model-collections/military-weapons
   • TurboSquid: https://www.turbosquid.com/Search/Index.cfm?keyword=military+weapons

"@
    
    $weaponsAssets | Out-File -FilePath (Join-Path $DownloadPath "MANUAL_DOWNLOADS_WEAPONS.txt") -Encoding UTF8
    Write-Host "✓ Created weapons asset list: MANUAL_DOWNLOADS_WEAPONS.txt`n" -ForegroundColor Green
}

# ═══════════════════════════════════════════════════════════════
# 6. AUTOMATED DOWNLOADS (Open Source Models)
# ═══════════════════════════════════════════════════════════════

Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
Write-Host "6. DOWNLOADING OPEN-SOURCE MODELS" -ForegroundColor Cyan
Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White

# GitHub repositories with open-source drone/robot models
$openSourceRepos = @(
    @{
        Url         = "https://github.com/PX4/PX4-SITL_gazebo-classic/archive/refs/heads/main.zip"
        File        = "PX4_Gazebo_Models.zip"
        Category    = "OpenSource\PX4"
        Description = "PX4 Autopilot Gazebo models (quadcopters, planes, rovers)"
    },
    @{
        Url         = "https://github.com/osrf/gazebo_models/archive/refs/heads/master.zip"
        File        = "Gazebo_Models.zip"
        Category    = "OpenSource\Gazebo"
        Description = "Gazebo robot models collection"
    },
    @{
        Url         = "https://github.com/ethz-asl/rotors_simulator/archive/refs/heads/master.zip"
        File        = "RotorS_UAV_Models.zip"
        Category    = "OpenSource\RotorS"
        Description = "ETH Zurich MAV/UAV simulator models"
    },
    @{
        Url         = "https://github.com/CopterExpress/clover/archive/refs/heads/master.zip"
        File        = "Clover_Drone.zip"
        Category    = "OpenSource\Clover"
        Description = "Clover quadcopter educational drone"
    }
)

foreach ($repo in $openSourceRepos) {
    Download-Asset -Url $repo.Url -FileName $repo.File -Category $repo.Category -Description $repo.Description
}

# ═══════════════════════════════════════════════════════════════
# 7. CESIUM TERRAIN & PHOTOGRAMMETRY
# ═══════════════════════════════════════════════════════════════

Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
Write-Host "7. REAL-WORLD TERRAIN SETUP" -ForegroundColor Cyan
Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White

$terrainGuide = @"
REAL-WORLD TERRAIN FOR AUTONOMYSIM
═══════════════════════════════════════════════════════════════

METHOD 1: CESIUM FOR UNREAL (RECOMMENDED)
──────────────────────────────────────────

1. Install Cesium for Unreal Plugin
   • https://cesium.com/platform/cesium-for-unreal/
   • Free plugin for Unreal Engine 5
   • Provides real-world 3D terrain

2. Get Cesium Ion Access Token
   • Sign up: https://cesium.com/ion/
   • Create new token
   • Configure in Unreal Editor

3. Stream Ukraine/Conflict Zone Terrain
   • Crimea Peninsula
   • Donbas Region (Donetsk, Luhansk)
   • Mariupol coastal area
   • Any location worldwide

4. Add Photogrammetry
   • Google Photorealistic 3D Tiles
   • Bing Maps 3D
   • High-resolution satellite imagery

METHOD 2: QUIXEL MEGASCANS (FREE)
──────────────────────────────────────────

1. Epic Games Account Required
   • https://quixel.com/megascans
   • Free with Unreal Engine

2. Download Asset Categories
   • Military structures
   • Damaged buildings
   • Natural environments
   • Props and vegetation

3. Quixel Bridge Integration
   • Direct import to Unreal
   • PBR materials
   • LOD support

METHOD 3: OPENSTREETMAP TO 3D
──────────────────────────────────────────

1. OSM to Unreal Plugin
   • https://github.com/ue4plugins/StreetMap
   • Convert OpenStreetMap data to 3D

2. Export Ukraine Cities
   • Kyiv, Kharkiv, Odesa, Lviv
   • https://www.openstreetmap.org/export

3. Generate Buildings
   • Automatic building generation
   • Road networks
   • Terrain features

METHOD 4: DEM/SATELLITE TERRAIN
──────────────────────────────────────────

1. USGS Earth Explorer
   • https://earthexplorer.usgs.gov/
   • Free DEM (Digital Elevation Model)
   • 30m resolution worldwide

2. Import to Unreal
   • World Machine (heightmap generation)
   • Gaea (terrain authoring)
   • Direct import to Unreal Landscape

3. Overlay Satellite Imagery
   • Google Earth imagery
   • Sentinel-2 (10m resolution)
   • Planet Labs (commercial)

SPECIFIC UKRAINE CONFLICT ZONES:
──────────────────────────────────────────

High-Value Target Locations:
• Crimean Bridge (Kerch Strait) - 45.3°N, 36.5°E
• Sevastopol Naval Base - 44.6°N, 33.5°E
• Donbas Urban Areas - 48.0°N, 37.8°E
• Dnipro River Crossings
• Antonovsky Bridge (Kherson)
• Black Sea Coast

Terrain Types Needed:
• Urban (destroyed buildings)
• Agricultural (open fields)
• Forest (tree cover)
• Coastal (naval operations)
• Industrial (factories, refineries)

"@

$terrainGuide | Out-File -FilePath (Join-Path $DownloadPath "TERRAIN_SETUP_GUIDE.txt") -Encoding UTF8
Write-Host "✓ Created terrain setup guide: TERRAIN_SETUP_GUIDE.txt`n" -ForegroundColor Green

# ═══════════════════════════════════════════════════════════════
# 8. INTEGRATION INSTRUCTIONS
# ═══════════════════════════════════════════════════════════════

Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
Write-Host "8. CREATING INTEGRATION GUIDE" -ForegroundColor Cyan
Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White

$integrationGuide = @"
AUTONOMYSIM ASSET INTEGRATION GUIDE
═══════════════════════════════════════════════════════════════

STEP 1: PREPARE UNREAL PROJECT
──────────────────────────────────────────

1. Open AutonomySim Unreal Project
   • Launch Unreal Engine 5
   • Open: Unreal\Environments\Blocks\Blocks.uproject

2. Enable Required Plugins
   • Cesium for Unreal
   • Quixel Bridge
   • Edit → Plugins → Search and Enable

STEP 2: IMPORT 3D MODELS
──────────────────────────────────────────

1. FBX/OBJ Import
   • Content Browser → Import
   • Select downloaded .fbx/.obj files
   • Configure import settings:
     - Enable "Import Skeletal Mesh" for articulated models
     - Set up materials and textures
     - Generate collision

2. Configure Physics
   • Set mass (UAV: 2-5kg, UGV: 50-500kg)
   • Add collision meshes
   • Configure physics materials

3. Set Up Vehicle Pawn
   • Create Blueprint from imported mesh
   • Add AutonomySim components:
     - MultiRotorPawnSimple (UAV)
     - CarPawn (UGV)
     - Custom vehicle type

STEP 3: CONFIGURE AUTONOMYSIM SETTINGS
──────────────────────────────────────────

1. Edit settings.json
   Location: ~/Documents/AutonomySim/settings.json

2. Add Vehicle Configuration:

{
  "SettingsVersion": 1.2,
  "SimMode": "Multirotor",
  "Vehicles": {
    "Bayraktar_TB2": {
      "VehicleType": "SimpleFlight",
      "X": 0, "Y": 0, "Z": -2,
      "Yaw": 0,
      "Cameras": {
        "front_center": {
          "CaptureSettings": [
            {"ImageType": 0, "Width": 1920, "Height": 1080}
          ]
        }
      }
    },
    "DJI_Mavic": {
      "VehicleType": "SimpleFlight",
      "X": 10, "Y": 0, "Z": -2
    },
    "Ground_Robot": {
      "VehicleType": "PhysXCar",
      "X": 0, "Y": 10, "Z": -1
    }
  }
}

STEP 4: ADD SENSORS
──────────────────────────────────────────

1. Camera Sensors
   • RGB Camera
   • Infrared/Thermal
   • Night vision

2. Lidar Configuration
   • Velodyne VLP-16
   • Ouster OS1-64

3. Additional Sensors
   • GPS/IMU
   • Magnetometer
   • Barometer

STEP 5: TEST VEHICLES
──────────────────────────────────────────

1. Python API Test:

import autonomysim as asim

client = asim.MultirotorClient()
client.confirmConnection()
client.enableApiControl(True)
client.armDisarm(True)
client.takeoffAsync().join()

# Hover
client.moveToPositionAsync(0, 0, -10, 5).join()

2. Formation Flight Test (Multiple UAVs):

from autonomysim.modules.ai import AgenticSwarmController

swarm = AgenticSwarmController()
swarm.addAgent("Bayraktar_TB2")
swarm.addAgent("DJI_Mavic_1")
swarm.addAgent("DJI_Mavic_2")
swarm.setFormation(FormationType.Wedge)

STEP 6: MISSION SCENARIOS
──────────────────────────────────────────

1. Reconnaissance Mission
   • Scout UAVs survey area
   • Transmit imagery
   • Coordinate with ground forces

2. Strike Mission
   • Loitering munition patrol
   • Target identification
   • Precision strike

3. Logistics
   • UGV supply transport
   • Autonomous navigation
   • Obstacle avoidance

4. Multi-Domain Operation
   • UAV air support
   • UGV ground advance
   • USV coastal patrol
   • Coordinated swarm attack

RECOMMENDED MODEL CONVERSIONS:
──────────────────────────────────────────

1. Sketchfab → Unreal
   • Download as FBX
   • Import to Blender
   • Clean geometry
   • Re-export with correct scale

2. CAD (STEP/IGES) → Unreal
   • Use Datasmith plugin
   • Or convert via Blender

3. Optimization
   • Reduce polygon count (LODs)
   • Optimize textures (2K max for real-time)
   • Bake lighting

UKRAINE SCENARIO EXAMPLE:
──────────────────────────────────────────

Mission: Reconnaissance of Russian Position

Vehicles:
• 3x DJI Mavic (recon)
• 1x Bayraktar TB2 (overwatch)
• 2x FPV drones (close inspection)
• 1x Ground robot (forward observation)

Environment:
• Eastern Ukraine terrain (Cesium)
• Destroyed urban area (Quixel)
• Fortified positions

Task:
• Coordinate swarm search pattern
• Identify targets
• Transmit real-time video
• Coordinate artillery strike

"@

$integrationGuide | Out-File -FilePath (Join-Path $DownloadPath "INTEGRATION_GUIDE.txt") -Encoding UTF8
Write-Host "✓ Created integration guide: INTEGRATION_GUIDE.txt`n" -ForegroundColor Green

# ═══════════════════════════════════════════════════════════════
# FINAL SUMMARY
# ═══════════════════════════════════════════════════════════════

Write-Host "`n╔═══════════════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║            DOWNLOAD PREPARATION COMPLETE                   ║" -ForegroundColor Green
Write-Host "╚═══════════════════════════════════════════════════════════════╝`n" -ForegroundColor Green

Write-Host "📋 CREATED DOCUMENTATION:" -ForegroundColor Cyan
Write-Host "  ✓ MANUAL_DOWNLOADS_ENVIRONMENTS.txt - High-fidelity environments" -ForegroundColor White
Write-Host "  ✓ MANUAL_DOWNLOADS_UAV.txt - Ukraine conflict drones & military UAVs" -ForegroundColor White
Write-Host "  ✓ MANUAL_DOWNLOADS_UGV.txt - Ground robots and combat vehicles" -ForegroundColor White
Write-Host "  ✓ MANUAL_DOWNLOADS_USV_UUV.txt - Maritime drones (surface & underwater)" -ForegroundColor White
Write-Host "  ✓ MANUAL_DOWNLOADS_WEAPONS.txt - Weapons & munitions assets" -ForegroundColor White
Write-Host "  ✓ TERRAIN_SETUP_GUIDE.txt - Real-world terrain setup (Cesium, OSM)" -ForegroundColor White
Write-Host "  ✓ INTEGRATION_GUIDE.txt - Complete integration instructions" -ForegroundColor White

Write-Host "`n📦 DOWNLOADED OPEN-SOURCE:" -ForegroundColor Cyan
Write-Host "  ✓ PX4 Gazebo Models (quadcopters, planes, rovers)" -ForegroundColor White
Write-Host "  ✓ Gazebo Robot Collection" -ForegroundColor White
Write-Host "  ✓ RotorS MAV/UAV Simulator Models" -ForegroundColor White
Write-Host "  ✓ Clover Educational Drone" -ForegroundColor White

Write-Host "`n🎯 KEY ASSETS DOCUMENTED:" -ForegroundColor Cyan
Write-Host "  • Bayraktar TB2 (Turkish attack drone)" -ForegroundColor Yellow
Write-Host "  • DJI Mavic 3 (recon & artillery spotting)" -ForegroundColor Yellow
Write-Host "  • DJI Phantom 4 Pro (munitions drop)" -ForegroundColor Yellow
Write-Host "  • Switchblade 300/600 (loitering munitions)" -ForegroundColor Yellow
Write-Host "  • Shahed-136 (Iranian kamikaze drone)" -ForegroundColor Yellow
Write-Host "  • FPV racing drones (modified for combat)" -ForegroundColor Yellow
Write-Host "  • Sea Baby/Magura V5 (Ukrainian naval drones)" -ForegroundColor Yellow
Write-Host "  • MQ-9 Reaper, Global Hawk (US drones)" -ForegroundColor Yellow

Write-Host "`n🌍 TERRAIN SOURCES:" -ForegroundColor Cyan
Write-Host "  • Cesium for Unreal (real-world 3D terrain)" -ForegroundColor White
Write-Host "  • Quixel Megascans (photogrammetry)" -ForegroundColor White
Write-Host "  • OpenStreetMap (city data)" -ForegroundColor White
Write-Host "  • USGS Earth Explorer (elevation data)" -ForegroundColor White

Write-Host "`n⚙️  NEXT STEPS:" -ForegroundColor Cyan
Write-Host "  1. Visit listed URLs and download desired assets" -ForegroundColor White
Write-Host "  2. Import 3D models into Unreal Engine" -ForegroundColor White
Write-Host "  3. Configure AutonomySim vehicle settings" -ForegroundColor White
Write-Host "  4. Set up Cesium for real-world terrain" -ForegroundColor White
Write-Host "  5. Test with Python API and swarm controller" -ForegroundColor White

Write-Host "`n💡 TIP: Start with PX4 Gazebo models for quick testing," -ForegroundColor Yellow
Write-Host "    then add high-fidelity models from Sketchfab/CGTrader.`n" -ForegroundColor Yellow

Write-Host "All documentation saved to: $DownloadPath`n" -ForegroundColor Green
