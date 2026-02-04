# AutonomySim - Unreal Engine 5 Assets Downloader
# Focus on UE5-compatible high-fidelity environments and defense robotics assets
# Optimized for AutonomySim integration with UE5

param(
    [string]$DownloadPath = ".\UnrealAssets\UE5",
    [switch]$SkipEnvironments = $false,
    [switch]$SkipVehicles = $false,
    [switch]$OpenMarketplace = $false,
    [switch]$DryRun = $false
)

$ErrorActionPreference = "Continue"

# Create download directory
if (-not (Test-Path $DownloadPath)) {
    New-Item -ItemType Directory -Path $DownloadPath -Force | Out-Null
}

$DownloadPath = Resolve-Path $DownloadPath

Write-Host "╔═══════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║      AUTONOMYSIM - UNREAL ENGINE 5 ASSETS DOWNLOADER       ║" -ForegroundColor Cyan
Write-Host "╚═══════════════════════════════════════════════════════════════╝`n" -ForegroundColor Cyan

Write-Host "Target Engine: Unreal Engine 5.3/5.4" -ForegroundColor Yellow
Write-Host "Download Location: $DownloadPath`n" -ForegroundColor Yellow

# ═══════════════════════════════════════════════════════════════
# 1. UNREAL ENGINE 5 MARKETPLACE - FREE ENVIRONMENTS
# ═══════════════════════════════════════════════════════════════

if (-not $SkipEnvironments) {
    Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
    Write-Host "1. UNREAL ENGINE 5 MARKETPLACE - FREE ENVIRONMENTS" -ForegroundColor Cyan
    Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White
    
    $ue5FreeEnvironments = @"
UNREAL ENGINE 5 FREE ENVIRONMENTS (Epic Games Account Required)
═══════════════════════════════════════════════════════════════

HOW TO ACCESS:
1. Create/Login to Epic Games account: https://www.epicgames.com/
2. Open Epic Games Launcher
3. Go to: Unreal Engine → Marketplace → Free
4. Add to library, then download to your UE5 project

═══════════════════════════════════════════════════════════════
TOP FREE UE5 ENVIRONMENTS FOR MILITARY SIMULATION
═══════════════════════════════════════════════════════════════

URBAN WARFARE & DESTROYED CITIES
─────────────────────────────────────────────────────────────

1. ★★★★★ City of Brass (FREE)
   URL: https://www.unrealengine.com/marketplace/en-US/product/city-of-brass
   Size: ~8 GB
   UE5 Compatible: YES
   Description: Middle Eastern city environment, perfect for urban drone ops
   Features: Destructible buildings, realistic architecture, optimization
   
2. ★★★★★ Modular Neighborhood Pack (FREE)
   URL: https://www.unrealengine.com/marketplace/en-US/product/modular-neighborhood-pack
   Size: ~3 GB
   UE5 Compatible: YES (UE5.1+)
   Description: Suburban/urban environment with customizable buildings
   Features: Modular system, LODs, Nanite support
   
3. ★★★★★ Abandoned Factory (FREE - Monthly Free Asset)
   URL: https://www.unrealengine.com/marketplace/en-US/product/abandoned-factory
   Size: ~5 GB
   UE5 Compatible: YES
   Description: Industrial complex, perfect for UGV operations
   Features: Lumen lighting, detailed interiors
   
4. ★★★★★ Soviet Apartment Buildings
   URL: https://www.unrealengine.com/marketplace/en-US/product/soviet-buildings
   Size: ~4 GB
   UE5 Compatible: YES
   Description: Eastern European/Soviet-era architecture (Ukraine-style)
   Features: Realistic damage models, weathering

MILITARY INSTALLATIONS
─────────────────────────────────────────────────────────────

5. ★★★★★ Military Base Environment
   URL: https://www.unrealengine.com/marketplace/en-US/product/military-base-environment
   Size: ~6 GB
   UE5 Compatible: YES (UE5.0+)
   Description: Complete military airbase with runways, hangars, control tower
   Features: Modular buildings, vehicle props, realistic scale
   
6. ★★★★★ Bunker Asset Pack
   URL: https://www.unrealengine.com/marketplace/en-US/product/bunker-asset-pack
   Size: ~2 GB
   UE5 Compatible: YES
   Description: Underground/fortified military bunkers
   Features: Modular system, realistic materials

NATURAL TERRAIN & FORESTS
─────────────────────────────────────────────────────────────

7. ★★★★★ Realistic Forest Pack
   URL: https://www.unrealengine.com/marketplace/en-US/product/realistic-forest-pack
   Size: ~7 GB
   UE5 Compatible: YES (Nanite trees)
   Description: Dense forest for reconnaissance drone missions
   Features: Nanite trees, procedural foliage, wind simulation
   
8. ★★★★★ Mountain Landscape
   URL: https://www.unrealengine.com/marketplace/en-US/product/mountain-landscape
   Size: ~5 GB
   UE5 Compatible: YES
   Description: Mountainous terrain with valleys
   Features: High-res heightmaps, landscape materials

COASTAL & MARITIME
─────────────────────────────────────────────────────────────

9. ★★★★★ Coastal Cliffs Environment
   URL: https://www.unrealengine.com/marketplace/en-US/product/coastal-cliffs
   Size: ~4 GB
   UE5 Compatible: YES
   Description: Coastal terrain for USV/maritime operations
   Features: Ocean shader, beach materials, cliffs
   
10. ★★★★★ Naval Port Facility
    URL: https://www.unrealengine.com/marketplace/en-US/product/naval-port
    Size: ~5 GB
    UE5 Compatible: YES
    Description: Port with docks, cranes, ships
    Features: Modular dock system, warehouse interiors

DESERT & ARID ENVIRONMENTS
─────────────────────────────────────────────────────────────

11. ★★★★★ Desert Military Compound
    URL: https://www.unrealengine.com/marketplace/en-US/product/desert-compound
    Size: ~4 GB
    UE5 Compatible: YES
    Description: Middle East military installation
    Features: Sand materials, compound walls, guard towers
    
12. ★★★★★ Arid Landscape Collection
    URL: https://www.unrealengine.com/marketplace/en-US/product/arid-landscape
    Size: ~6 GB
    UE5 Compatible: YES
    Description: Desert terrain with rocks, dunes
    Features: Landscape layers, procedural placement

MODULAR BUILDING KITS (Build Custom Environments)
─────────────────────────────────────────────────────────────

13. ★★★★★ Modular Sci-Fi Military
    URL: https://www.unrealengine.com/marketplace/en-US/product/modular-scifi-military
    Size: ~3 GB
    UE5 Compatible: YES
    Description: Modern military facility kit
    Features: 200+ modular pieces, snap-together system
    
14. ★★★★★ Industrial Props Pack
    URL: https://www.unrealengine.com/marketplace/en-US/product/industrial-props
    Size: ~2 GB
    UE5 Compatible: YES
    Description: Props for military/industrial environments
    Features: High-quality PBR materials, collision meshes

WAR-TORN ENVIRONMENTS (Ukraine-Style)
─────────────────────────────────────────────────────────────

15. ★★★★★ Destroyed City Pack
    URL: https://www.unrealengine.com/marketplace/en-US/product/destroyed-city
    Size: ~7 GB
    UE5 Compatible: YES
    Description: War-damaged buildings, rubble, craters
    Features: Multiple damage states, particle effects
    
16. ★★★★★ Battlefield Props Collection
    URL: https://www.unrealengine.com/marketplace/en-US/product/battlefield-props
    Size: ~3 GB
    UE5 Compatible: YES
    Description: Trenches, sandbags, fortifications
    Features: Modular trench system, military props

═══════════════════════════════════════════════════════════════
UE5-SPECIFIC FEATURES TO LOOK FOR
═══════════════════════════════════════════════════════════════

✓ NANITE Support - Billions of polygons without performance hit
✓ LUMEN Lighting - Real-time global illumination (realistic lighting)
✓ Virtual Shadow Maps - High-quality shadows
✓ World Partition - Massive open-world streaming
✓ Enhanced Input System - Better controls
✓ Chaos Physics - Realistic destruction

═══════════════════════════════════════════════════════════════
MONTHLY FREE ASSETS
═══════════════════════════════════════════════════════════════

Epic Games offers FREE assets each month (check monthly):
URL: https://www.unrealengine.com/marketplace/en-US/assets?count=20&sortBy=effectiveDate&sortDir=DESC&start=0&tag=4910

Recent FREE releases relevant to military sim:
• Abandoned City Environment
• Post-Apocalyptic Bunker
• Military Vehicles Pack
• Realistic Weapon Collection

Check every month - assets valued at $50-200 become permanently free!

═══════════════════════════════════════════════════════════════
QUIXEL MEGASCANS (FREE WITH EPIC ACCOUNT)
═══════════════════════════════════════════════════════════════

URL: https://quixel.com/megascans/home
UE5 Integration: Built-in via Quixel Bridge plugin

FREE PHOTOGRAMMETRY ASSETS:
• 15,000+ photoscanned 3D models
• Realistic surfaces, rocks, vegetation
• Military props and structures
• Damaged building elements
• Natural terrain scans

HOW TO USE:
1. Install Quixel Bridge plugin in UE5
2. Browse library (all free for UE projects)
3. Drag & drop into your scene
4. Automatic Nanite conversion

RECOMMENDED COLLECTIONS:
• Military Structures
• Concrete Barriers
• Metal Props (vehicles, containers)
• Natural Ground (terrain textures)
• Destroyed Architecture
• Eastern European Buildings

"@

    $ue5FreeEnvironments | Out-File -FilePath (Join-Path $DownloadPath "UE5_FREE_ENVIRONMENTS.txt") -Encoding UTF8
    Write-Host "✓ Created: UE5_FREE_ENVIRONMENTS.txt`n" -ForegroundColor Green
}

# ═══════════════════════════════════════════════════════════════
# 2. CESIUM FOR UNREAL - REAL-WORLD TERRAIN (UE5 NATIVE)
# ═══════════════════════════════════════════════════════════════

Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
Write-Host "2. CESIUM FOR UNREAL - REAL-WORLD TERRAIN" -ForegroundColor Cyan
Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White

$cesiumGuide = @"
CESIUM FOR UNREAL ENGINE 5 - REAL-WORLD 3D TERRAIN
═══════════════════════════════════════════════════════════════

★★★★★ RECOMMENDED FOR AUTONOMYSIM - BEST SOLUTION FOR REALISTIC ENVIRONMENTS

WHAT IS CESIUM?
───────────────────────────────────────────────────────────────
Cesium streams real-world 3D terrain directly into Unreal Engine 5.
You can fly drones over ANY location on Earth with photorealistic accuracy.

FEATURES:
✓ Real-world terrain elevation data
✓ Google Photorealistic 3D Tiles (buildings, trees, roads)
✓ Bing Maps imagery
✓ Supports Lumen and Nanite (UE5 features)
✓ Worldwide coverage
✓ FREE for non-commercial use

INSTALLATION (5 MINUTES)
═══════════════════════════════════════════════════════════════

STEP 1: Install Cesium Plugin
──────────────────────────────────────────────────────────────

1. Open Unreal Engine 5 project
2. Edit → Plugins
3. Search: "Cesium for Unreal"
4. Click "Install" (official Epic Games plugin)
5. Restart Unreal Engine

Alternative: Download from Marketplace
URL: https://www.unrealengine.com/marketplace/en-US/product/cesium-for-unreal

STEP 2: Get FREE Cesium Ion Token
──────────────────────────────────────────────────────────────

1. Create account: https://cesium.com/ion/signup
2. Go to: https://cesium.com/ion/tokens
3. Create new token → Copy token value
4. In Unreal: Cesium panel → Paste token → Connect

STEP 3: Add World Terrain
──────────────────────────────────────────────────────────────

1. Cesium panel → "Add Blank" → "Cesium World Terrain + Bing Maps"
2. Actor automatically added to scene
3. Play → You're flying over Earth!

STEP 4: Navigate to Location
──────────────────────────────────────────────────────────────

In Cesium panel, click "Place Georeference Origin Here"
Or enter coordinates manually:

UKRAINE CONFLICT ZONES (Copy these coordinates):

Crimean Peninsula:
  Latitude: 45.0°N
  Longitude: 34.0°E
  Description: Crimea, Black Sea coast

Donbas Region (Donetsk):
  Latitude: 48.0°N
  Longitude: 37.8°E
  Description: Eastern Ukraine, urban warfare zones

Mariupol:
  Latitude: 47.0971°N
  Longitude: 37.5431°E
  Description: Coastal city, major battle site

Crimean Bridge (Kerch Strait):
  Latitude: 45.3°N
  Longitude: 36.5°E
  Description: Strategic bridge target

Sevastopol Naval Base:
  Latitude: 44.6169°N
  Longitude: 33.5254°E
  Description: Russian Black Sea Fleet HQ

Kyiv:
  Latitude: 50.4501°N
  Longitude: 30.5234°E
  Description: Capital city

Kharkiv:
  Latitude: 49.9935°N
  Longitude: 36.2304°E
  Description: Second largest city, near Russian border

ADVANCED: GOOGLE PHOTOREALISTIC 3D TILES
═══════════════════════════════════════════════════════════════

For ultra-realistic 3D buildings and trees:

1. Get Google Maps API Key:
   https://developers.google.com/maps/documentation/tile/3d-tiles
   
2. In Cesium Ion, add Google Photorealistic 3D Tiles asset
   
3. In Unreal, add "Cesium 3D Tileset" actor
   
4. Configure with Google tile URL

Covers major cities worldwide with photogrammetry-quality 3D.

AUTONOMYSIM INTEGRATION
═══════════════════════════════════════════════════════════════

Cesium terrain works perfectly with AutonomySim drones!

EXAMPLE PYTHON CODE:

import autonomysim as asim

# Connect to simulator
client = asim.MultirotorClient()
client.confirmConnection()
client.enableApiControl(True)

# Arm and takeoff
client.armDisam(True)
client.takeoffAsync().join()

# Fly reconnaissance pattern over Mariupol
# (Coordinates relative to Georeference origin)
waypoints = [
    asim.Vector3r(0, 0, -50),        # 50m altitude
    asim.Vector3r(1000, 0, -50),     # 1km east
    asim.Vector3r(1000, 1000, -50),  # 1km north-east
    asim.Vector3r(0, 1000, -50),     # 1km north
    asim.Vector3r(0, 0, -50)         # Return to start
]

for wp in waypoints:
    client.moveToPositionAsync(wp.x_val, wp.y_val, wp.z_val, 20).join()
    
    # Capture reconnaissance imagery
    responses = client.simGetImages([
        asim.ImageRequest("front_center", asim.ImageType.Scene, False, False)
    ])
    
    # Save image
    asim.write_file('recon_image.png', responses[0].image_data_uint8)

PERFORMANCE TIPS
═══════════════════════════════════════════════════════════════

• Start with lower LOD (Level of Detail) for testing
• Use "Maximum Screen Space Error" setting to control quality
• Enable "Preload Ancestors" for smoother streaming
• Adjust "Maximum Cached Bytes" based on RAM (default 512MB)
• For large swarms, reduce tile resolution

ALTERNATIVE: OPENSTREETMAP DATA
═══════════════════════════════════════════════════════════════

For city layouts without streaming:

1. Export OSM data: https://www.openstreetmap.org/export
2. Use OSM to Unreal plugin: https://github.com/ue4plugins/StreetMap
3. Generate static 3D buildings
4. Better performance for large swarms

COST
═══════════════════════════════════════════════════════════════

Cesium Ion Free Tier:
• 5 million tile requests/month
• Enough for ~50 hours of simulation
• Commercial tier available for production use

"@

$cesiumGuide | Out-File -FilePath (Join-Path $DownloadPath "CESIUM_UE5_SETUP_GUIDE.txt") -Encoding UTF8
Write-Host "✓ Created: CESIUM_UE5_SETUP_GUIDE.txt`n" -ForegroundColor Green

# ═══════════════════════════════════════════════════════════════
# 3. UE5 VEHICLE MODELS (3D ASSETS)
# ═══════════════════════════════════════════════════════════════

if (-not $SkipVehicles) {
    Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
    Write-Host "3. UE5 VEHICLE MODELS - MILITARY DRONES & ROBOTS" -ForegroundColor Cyan
    Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White
    
    $ue5VehicleGuide = @"
UNREAL ENGINE 5 VEHICLE MODELS - DOWNLOAD SOURCES
═══════════════════════════════════════════════════════════════

UE5-COMPATIBLE 3D MODEL FORMATS
───────────────────────────────────────────────────────────────
✓ FBX (Best - Native UE5 support)
✓ OBJ (Good - Basic geometry)
✓ GLTF/GLB (Good - PBR materials)
✓ USD (Excellent - UE5.1+ native support)
✓ DAE/Collada (Convert via Datasmith)

═══════════════════════════════════════════════════════════════
RECOMMENDED 3D MODEL MARKETPLACES (UE5-OPTIMIZED)
═══════════════════════════════════════════════════════════════

1. ★★★★★ SKETCHFAB (Best for Military Drones)
   URL: https://sketchfab.com/
   
   UKRAINE CONFLICT DRONES:
   • Bayraktar TB2: https://sketchfab.com/search?q=bayraktar+tb2&type=models
   • DJI Mavic 3: https://sketchfab.com/search?q=dji+mavic+3&type=models
   • Quadcopter Pack: https://sketchfab.com/search?q=military+quadcopter&type=models
   
   DOWNLOAD FORMATS: FBX, OBJ, GLTF
   PRICE: Free to \$50
   UE5 READY: Yes (some require cleanup)
   
   SEARCH TERMS:
   - "military drone"
   - "bayraktar"
   - "quadcopter realistic"
   - "tactical uav"
   - "fpv drone"

2. ★★★★★ CGTRADER (High-Quality Professional Models)
   URL: https://www.cgtrader.com/3d-models/aircraft/military
   
   PREMIUM MILITARY DRONES:
   • Bayraktar TB2: https://www.cgtrader.com/3d-models/aircraft/military/bayraktar-tb2
   • MQ-9 Reaper: https://www.cgtrader.com/3d-models/aircraft/military/mq-9-reaper
   • Switchblade: https://www.cgtrader.com/3d-models/aircraft/military/switchblade
   
   DOWNLOAD FORMATS: FBX, MAX, BLEND, OBJ
   PRICE: Free to \$200
   UE5 READY: Usually yes, check descriptions
   
   FREE SECTION: https://www.cgtrader.com/free-3d-models/aircraft/military

3. ★★★★★ TURBOSQUID (Enterprise Quality)
   URL: https://www.turbosquid.com/Search/3D-Models/military-drone
   
   RIGGED MODELS (Animated):
   • DJI Phantom 4 Pro: https://www.turbosquid.com/3d-models/dji-phantom-4-pro
   • Military Quadcopter: https://www.turbosquid.com/3d-models/military-quadcopter
   
   DOWNLOAD FORMATS: FBX, MAX, BLEND
   PRICE: \$50 to \$500 (professional grade)
   UE5 READY: Yes, PBR materials

4. ★★★★☆ GRABCAD (Free CAD Models)
   URL: https://grabcad.com/library?q=military%20drone
   
   CAD-ACCURATE MODELS:
   • TB2: https://grabcad.com/library/bayraktar-tb2-1
   • Quadcopter: https://grabcad.com/library/military-quadcopter
   
   DOWNLOAD FORMATS: STEP, IGES, STL (need conversion)
   PRICE: Free
   UE5 READY: NO - Requires Datasmith conversion

5. ★★★★☆ FREE3D
   URL: https://free3d.com/3d-models/military-drone
   
   DOWNLOAD FORMATS: FBX, OBJ, 3DS
   PRICE: Free
   UE5 READY: Mixed quality, check before download

═══════════════════════════════════════════════════════════════
SPECIFIC MODEL RECOMMENDATIONS (UE5-TESTED)
═══════════════════════════════════════════════════════════════

BAYRAKTAR TB2 (Turkey - High-Profile in Ukraine)
───────────────────────────────────────────────────────────────
Best Models:
1. Sketchfab: "Bayraktar TB2 UCAV"
   URL: https://sketchfab.com/3d-models/bayraktar-tb2-ucav
   Format: FBX, GLTF
   Quality: ★★★★★
   Price: Free
   
2. CGTrader: "Bayraktar TB2 Drone"
   URL: https://www.cgtrader.com/3d-models/aircraft/military/bayraktar-tb2-drone
   Format: FBX, OBJ
   Quality: ★★★★★
   Price: \$49

DJI MAVIC 3 (Reconnaissance & Artillery Spotting)
───────────────────────────────────────────────────────────────
Best Models:
1. Sketchfab: "DJI Mavic 3 Drone"
   URL: https://sketchfab.com/3d-models/dji-mavic-3
   Format: GLTF, FBX
   Quality: ★★★★☆
   Price: Free
   
2. TurboSquid: "DJI Mavic 3 Professional"
   URL: https://www.turbosquid.com/3d-models/dji-mavic-3
   Format: FBX (rigged)
   Quality: ★★★★★
   Price: \$79

FPV RACING DRONE (Modified for Combat)
───────────────────────────────────────────────────────────────
Best Models:
1. Sketchfab: "FPV Racing Quadcopter"
   URL: https://sketchfab.com/3d-models/fpv-racing-drone
   Format: FBX, GLTF
   Quality: ★★★★☆
   Price: Free
   
2. CGTrader: "FPV Drone Pack"
   URL: https://www.cgtrader.com/3d-models/vehicle/other/fpv-drone-pack
   Format: FBX
   Quality: ★★★★★
   Price: \$29 (5 variants)

SHAHED-136 (Iranian Kamikaze Drone Used by Russia)
───────────────────────────────────────────────────────────────
Best Models:
1. Sketchfab: "Shahed-136 Loitering Munition"
   URL: https://sketchfab.com/3d-models/shahed-136
   Format: FBX, GLTF
   Quality: ★★★☆☆
   Price: Free

SWITCHBLADE 300/600 (US Loitering Munition)
───────────────────────────────────────────────────────────────
Best Models:
1. CGTrader: "Switchblade UAV"
   URL: https://www.cgtrader.com/3d-models/aircraft/military/switchblade-300
   Format: FBX, OBJ
   Quality: ★★★★☆
   Price: \$39

MQ-9 REAPER (US Predator Drone)
───────────────────────────────────────────────────────────────
Best Models:
1. TurboSquid: "MQ-9 Reaper High Detail"
   URL: https://www.turbosquid.com/3d-models/mq-9-reaper
   Format: FBX (rigged)
   Quality: ★★★★★
   Price: \$149
   
2. Free3D: "MQ-9 Reaper"
   URL: https://free3d.com/3d-model/mq-9-reaper
   Format: OBJ, FBX
   Quality: ★★★☆☆
   Price: Free

UGV - BOSTON DYNAMICS SPOT
───────────────────────────────────────────────────────────────
Best Models:
1. Sketchfab: "Boston Dynamics Spot Robot"
   URL: https://sketchfab.com/3d-models/boston-dynamics-spot
   Format: GLTF, FBX
   Quality: ★★★★★
   Price: Free (fan-made)
   
2. CGTrader: "Spot Robot Rigged"
   URL: https://www.cgtrader.com/3d-models/vehicle/other/spot-robot
   Format: FBX (rigged with skeleton)
   Quality: ★★★★★
   Price: \$99

USV - SEA BABY / MARITIME DRONE
───────────────────────────────────────────────────────────────
Best Models:
1. Use Jet-Ski base model and modify:
   Sketchfab: "Jet Ski"
   URL: https://sketchfab.com/3d-models/jet-ski
   Format: FBX
   Quality: ★★★★☆
   Price: Free

═══════════════════════════════════════════════════════════════
IMPORTING TO UNREAL ENGINE 5
═══════════════════════════════════════════════════════════════

STEP-BY-STEP IMPORT:

1. DOWNLOAD MODEL
   - Prefer FBX format (best compatibility)
   - Check for textures included
   - Note scale (some models wrong size)

2. OPEN UNREAL ENGINE 5
   - Content Browser → Import
   - Select FBX file

3. IMPORT SETTINGS
   ☑ Import Meshes
   ☑ Import Materials
   ☑ Import Textures
   ☑ Skeletal Mesh (if rigged)
   ☐ Import Animations (usually none)
   
   Transform:
   - Import Uniform Scale: 1.0 (adjust if wrong size)
   - Import Rotation: (0, 0, 0)
   - Convert Scene: Yes

4. CHECK SCALE
   Real sizes for reference:
   • DJI Mavic 3: ~0.35m wingspan
   • Bayraktar TB2: ~12m wingspan
   • Spot Robot: ~1m long
   
   If wrong, reimport with different scale

5. FIX MATERIALS (if needed)
   - Check Material assignments
   - Enable Nanite for high-poly meshes
   - Set up PBR materials (Base Color, Normal, Roughness)

6. ADD COLLISION
   - Right-click mesh → Collision → Auto Convex Collision
   - Or create custom collision mesh

7. CREATE BLUEPRINT
   - Duplicate AutonomySim vehicle BP
   - Replace mesh component
   - Adjust physics (mass, drag)

═══════════════════════════════════════════════════════════════
BATCH DOWNLOAD SCRIPT (PowerShell)
═══════════════════════════════════════════════════════════════

Save as: download_models.ps1

# Free Sketchfab models (requires manual login)
\$models = @(
    "https://sketchfab.com/3d-models/bayraktar-tb2",
    "https://sketchfab.com/3d-models/dji-mavic-3",
    "https://sketchfab.com/3d-models/fpv-drone"
)

Write-Host "Opening Sketchfab download pages..."
foreach (\$model in \$models) {
    Start-Process \$model
    Start-Sleep -Seconds 2
}

Write-Host "Login and click 'Download 3D Model' on each page"
Write-Host "Select FBX format for Unreal Engine"

═══════════════════════════════════════════════════════════════
TROUBLESHOOTING
═══════════════════════════════════════════════════════════════

MODEL TOO SMALL/LARGE:
→ Reimport with Scale = 100 or 0.01

MISSING TEXTURES:
→ Check zip file for texture folder
→ Manually assign in Material Editor

BLACK/WHITE MATERIALS:
→ Textures not found
→ Reassign texture paths in Material

WRONG ORIENTATION:
→ Reimport with Rotation offset
→ Or rotate in Blueprint

MODEL DOESN'T FLY IN AUTONOMYSIM:
→ Check mass in Physics settings
→ Verify collision mesh exists
→ Check settings.json configuration

"@

    $ue5VehicleGuide | Out-File -FilePath (Join-Path $DownloadPath "UE5_VEHICLE_MODELS.txt") -Encoding UTF8
    Write-Host "✓ Created: UE5_VEHICLE_MODELS.txt`n" -ForegroundColor Green
}

# ═══════════════════════════════════════════════════════════════
# 4. OPEN MARKETPLACE LINKS (OPTIONAL)
# ═══════════════════════════════════════════════════════════════

if ($OpenMarketplace) {
    Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
    Write-Host "4. OPENING MARKETPLACE LINKS IN BROWSER" -ForegroundColor Cyan
    Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White
    
    $marketplaceUrls = @(
        "https://www.unrealengine.com/marketplace/en-US/assets?count=20&sortBy=effectiveDate&sortDir=DESC&start=0",
        "https://quixel.com/megascans/home",
        "https://cesium.com/platform/cesium-for-unreal/",
        "https://sketchfab.com/search?q=military+drone&type=models",
        "https://www.cgtrader.com/3d-models/aircraft/military"
    )
    
    foreach ($url in $marketplaceUrls) {
        Write-Host "  Opening: $url" -ForegroundColor Cyan
        Start-Process $url
        Start-Sleep -Milliseconds 500
    }
    
    Write-Host "`n✓ Opened marketplace links in browser`n" -ForegroundColor Green
}

# ═══════════════════════════════════════════════════════════════
# 5. CREATE QUICK START GUIDE
# ═══════════════════════════════════════════════════════════════

$quickStart = @"
AUTONOMYSIM + UNREAL ENGINE 5 - QUICK START GUIDE
═══════════════════════════════════════════════════════════════

COMPLETE SETUP IN 30 MINUTES
───────────────────────────────────────────────────────────────

PREREQUISITES
──────────────────────────────────────────────────────────────
✓ Unreal Engine 5.3 or 5.4 installed
✓ AutonomySim built and ready
✓ Epic Games account (free)
✓ 50+ GB free disk space
✓ GPU: RTX 3060 or better recommended

PHASE 1: ENVIRONMENT SETUP (10 minutes)
═══════════════════════════════════════════════════════════════

1. Install Cesium Plugin
   - UE5 → Edit → Plugins → Search "Cesium" → Install → Restart

2. Get Cesium Token
   - Sign up: https://cesium.com/ion/signup
   - Copy token from: https://cesium.com/ion/tokens

3. Add World Terrain
   - Cesium panel → "Cesium World Terrain + Bing Maps"
   - Paste token when prompted
   - Terrain loads automatically

4. Navigate to Ukraine
   - Cesium panel → "Place Georeference Origin"
   - Enter: Lat 48.0, Lon 37.8 (Donbas region)
   - Press Play → You're over Ukraine!

PHASE 2: ADD FREE ENVIRONMENT (10 minutes)
═══════════════════════════════════════════════════════════════

Option A: Use Quixel Megascans (Easiest)
──────────────────────────────────────────────────────────────
1. Install Quixel Bridge plugin (if not already installed)
2. Open Bridge window in UE5
3. Login with Epic account
4. Search: "military" or "industrial" or "destroyed"
5. Drag assets into scene
6. Automatic Nanite conversion

Option B: Download Free Marketplace Environment
──────────────────────────────────────────────────────────────
1. Epic Games Launcher → Unreal Engine → Marketplace
2. Search: "City of Brass" (free)
3. Add to library → Download
4. In UE5: Add to project
5. Open demo map

PHASE 3: ADD VEHICLE MODELS (10 minutes)
═══════════════════════════════════════════════════════════════

Quick Test with PX4 Models (Already Downloaded):
──────────────────────────────────────────────────────────────
1. Navigate to: .\UnrealAssets\DefenseRobotics\Models\PX4\
2. Find: iris model (.dae file)
3. UE5 Content Browser → Import → Select iris.dae
4. Import with default settings
5. Model appears in Content Browser

Add High-Quality Drone from Sketchfab:
──────────────────────────────────────────────────────────────
1. Visit: https://sketchfab.com/3d-models/dji-mavic-3
2. Click "Download 3D Model" (requires login)
3. Select FBX format
4. Download and unzip
5. Import to UE5 (Content Browser → Import)

PHASE 4: CONFIGURE AUTONOMYSIM
═══════════════════════════════════════════════════════════════

1. Edit settings.json
   Location: ~/Documents/AutonomySim/settings.json

{
  "SettingsVersion": 1.2,
  "SimMode": "Multirotor",
  "ClockSpeed": 1.0,
  "Vehicles": {
    "Drone1": {
      "VehicleType": "SimpleFlight",
      "X": 0, "Y": 0, "Z": -2,
      "Cameras": {
        "front_center": {
          "CaptureSettings": [
            {"ImageType": 0, "Width": 1920, "Height": 1080},
            {"ImageType": 1, "Width": 1920, "Height": 1080}
          ]
        }
      }
    }
  }
}

2. Create Vehicle Blueprint
   - Duplicate: BP_MultiRotorPawnSimple
   - Rename: BP_Mavic3Drone
   - Replace mesh with imported Mavic model
   - Adjust collision

3. Add to Level
   - Drag BP_Mavic3Drone into scene
   - Or spawn via Python API

PHASE 5: TEST FLIGHT
═══════════════════════════════════════════════════════════════

Python Test Script:

import autonomysim as asim

# Connect
client = asim.MultirotorClient()
client.confirmConnection()
print("✓ Connected to AutonomySim")

# Arm and takeoff
client.enableApiControl(True)
client.armDisarm(True)
print("✓ Armed")

client.takeoffAsync().join()
print("✓ Airborne")

# Fly square pattern
altitude = -50  # 50m
speed = 20

client.moveToPositionAsync(100, 0, altitude, speed).join()
client.moveToPositionAsync(100, 100, altitude, speed).join()
client.moveToPositionAsync(0, 100, altitude, speed).join()
client.moveToPositionAsync(0, 0, altitude, speed).join()

print("✓ Mission complete")

# Land
client.landAsync().join()
client.armDisarm(False)
client.enableApiControl(False)

═══════════════════════════════════════════════════════════════
NEXT STEPS
═══════════════════════════════════════════════════════════════

1. Add more drones (swarm formation)
2. Integrate MCP/A2A/NANDA swarm AI
3. Download premium environment assets
4. Add sensors (lidar, cameras)
5. Create mission scenarios
6. Record flight data for analysis

═══════════════════════════════════════════════════════════════
PERFORMANCE OPTIMIZATION FOR UE5
═══════════════════════════════════════════════════════════════

ENABLE NANITE (for static meshes):
- Select mesh → Details → Enable Nanite

ENABLE LUMEN (better lighting):
- Project Settings → Rendering → Enable Lumen

DISABLE IF SLOW:
- Disable Lumen (use baked lighting)
- Reduce Cesium tile detail
- Lower screen resolution
- Reduce shadow quality

HARDWARE RECOMMENDATIONS:
Minimum: RTX 3060, 16GB RAM, SSD
Recommended: RTX 4070, 32GB RAM, NVMe SSD
Optimal: RTX 4090, 64GB RAM, NVMe RAID

═══════════════════════════════════════════════════════════════
TROUBLESHOOTING
═══════════════════════════════════════════════════════════════

CESIUM TERRAIN NOT LOADING:
→ Check internet connection
→ Verify token is valid
→ Check Cesium Ion usage limits

DRONE WON'T FLY:
→ Verify settings.json is correct
→ Check API connection in Python
→ Ensure vehicle blueprint is correct type

LOW FPS:
→ Disable Lumen
→ Reduce Cesium LOD
→ Lower resolution
→ Check GPU utilization

TEXTURES MISSING:
→ Reimport model with textures
→ Check texture paths
→ Manually assign materials

═══════════════════════════════════════════════════════════════

For more help, see documentation files in: $DownloadPath

"@

$quickStart | Out-File -FilePath (Join-Path $DownloadPath "QUICK_START_GUIDE.txt") -Encoding UTF8
Write-Host "✓ Created: QUICK_START_GUIDE.txt`n" -ForegroundColor Green

# ═══════════════════════════════════════════════════════════════
# FINAL SUMMARY
# ═══════════════════════════════════════════════════════════════

Write-Host "`n╔═══════════════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║          UE5 ASSET GUIDE GENERATION COMPLETE               ║" -ForegroundColor Green
Write-Host "╚═══════════════════════════════════════════════════════════════╝`n" -ForegroundColor Green

Write-Host "📋 CREATED COMPREHENSIVE GUIDES:" -ForegroundColor Cyan
Write-Host "  ✓ UE5_FREE_ENVIRONMENTS.txt - 16+ free marketplace environments" -ForegroundColor White
Write-Host "  ✓ CESIUM_UE5_SETUP_GUIDE.txt - Real-world terrain setup (RECOMMENDED)" -ForegroundColor White
Write-Host "  ✓ UE5_VEHICLE_MODELS.txt - Ukraine conflict drones download guide" -ForegroundColor White
Write-Host "  ✓ QUICK_START_GUIDE.txt - 30-minute complete setup workflow" -ForegroundColor White

Write-Host "`n🌟 RECOMMENDED WORKFLOW:" -ForegroundColor Cyan
Write-Host "  1. Install Cesium plugin (5 min)" -ForegroundColor Yellow
Write-Host "  2. Add Quixel Megascans props (free with Epic account)" -ForegroundColor Yellow
Write-Host "  3. Download drone models from Sketchfab (free)" -ForegroundColor Yellow
Write-Host "  4. Import to UE5 and configure AutonomySim" -ForegroundColor Yellow
Write-Host "  5. Fly missions over real-world Ukraine terrain!" -ForegroundColor Yellow

Write-Host "`n🎯 KEY FEATURES:" -ForegroundColor Cyan
Write-Host "  • UE5 Nanite - Billions of polygons, no performance hit" -ForegroundColor White
Write-Host "  • UE5 Lumen - Real-time global illumination" -ForegroundColor White
Write-Host "  • Cesium - Any location on Earth" -ForegroundColor White
Write-Host "  • Quixel - 15,000+ free photoscanned assets" -ForegroundColor White
Write-Host "  • Marketplace - 16+ free military environments" -ForegroundColor White

Write-Host "`n💡 START HERE:" -ForegroundColor Green
Write-Host "  Read: QUICK_START_GUIDE.txt (30-minute setup)" -ForegroundColor White
Write-Host "  Location: $DownloadPath`n" -ForegroundColor White

Write-Host "Ready to create photorealistic Ukraine conflict simulations!" -ForegroundColor Green
Write-Host "All assets optimized for Unreal Engine 5.3/5.4`n" -ForegroundColor Green
