# Extract and Organize Downloaded Defense Assets
# Prepares open-source models for Unreal Engine import

param(
    [string]$SourcePath = ".\UnrealAssets\DefenseRobotics\OpenSource",
    [string]$OutputPath = ".\UnrealAssets\DefenseRobotics\Models",
    [switch]$SkipExtraction = $false
)

$ErrorActionPreference = "Continue"

Write-Host "╔═══════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║     AUTONOMYSIM - ASSET EXTRACTION & ORGANIZATION          ║" -ForegroundColor Cyan
Write-Host "╚═══════════════════════════════════════════════════════════════╝`n" -ForegroundColor Cyan

# Create output directory
if (-not (Test-Path $OutputPath)) {
    New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
}

# ═══════════════════════════════════════════════════════════════
# 1. EXTRACT DOWNLOADED ARCHIVES
# ═══════════════════════════════════════════════════════════════

if (-not $SkipExtraction) {
    Write-Host "─────────────────────────────────────────────────────────────────" -ForegroundColor White
    Write-Host "1. EXTRACTING ARCHIVES" -ForegroundColor Cyan
    Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White
    
    $archives = @(
        @{
            Path        = "$SourcePath\PX4\PX4_Gazebo_Models.zip"
            Destination = "$OutputPath\PX4"
            Name        = "PX4 Gazebo Models"
        },
        @{
            Path        = "$SourcePath\Gazebo\Gazebo_Models.zip"
            Destination = "$OutputPath\Gazebo"
            Name        = "Gazebo Robot Models"
        },
        @{
            Path        = "$SourcePath\RotorS\RotorS_UAV_Models.zip"
            Destination = "$OutputPath\RotorS"
            Name        = "RotorS MAV/UAV Models"
        },
        @{
            Path        = "$SourcePath\Clover\Clover_Drone.zip"
            Destination = "$OutputPath\Clover"
            Name        = "Clover Educational Drone"
        }
    )
    
    foreach ($archive in $archives) {
        if (Test-Path $archive.Path) {
            Write-Host "  Extracting: $($archive.Name)" -ForegroundColor Cyan
            
            if (Test-Path $archive.Destination) {
                Write-Host "    Already extracted - skipping" -ForegroundColor Yellow
            }
            else {
                try {
                    Expand-Archive -Path $archive.Path -DestinationPath $archive.Destination -Force
                    Write-Host "    ✓ Extracted to: $($archive.Destination)" -ForegroundColor Green
                }
                catch {
                    Write-Host "    ✗ Failed: $($_.Exception.Message)" -ForegroundColor Red
                }
            }
        }
        else {
            Write-Host "  ✗ Not found: $($archive.Path)" -ForegroundColor Red
        }
    }
}

# ═══════════════════════════════════════════════════════════════
# 2. CATALOG 3D MODEL FILES
# ═══════════════════════════════════════════════════════════════

Write-Host "`n─────────────────────────────────────────────────────────────────" -ForegroundColor White
Write-Host "2. CATALOGING 3D MODELS" -ForegroundColor Cyan
Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White

$modelExtensions = @("*.dae", "*.stl", "*.obj", "*.fbx", "*.sdf", "*.urdf", "*.xacro")
$models = @()

foreach ($ext in $modelExtensions) {
    $found = Get-ChildItem -Path $OutputPath -Filter $ext -Recurse -ErrorAction SilentlyContinue
    if ($found) {
        $models += $found
    }
}

Write-Host "Found $($models.Count) 3D model files:`n" -ForegroundColor Green

# Group by category
$categories = @{
    "UAV"     = @("quadrotor", "iris", "plane", "vtol", "typhoon", "solo", "tailsitter", "matrice")
    "UGV"     = @("rover", "car", "vehicle", "ackermann", "ground", "husky", "jackal")
    "USV"     = @("boat", "ship", "maritime", "wamv", "usv")
    "UUV"     = @("submarine", "underwater", "auv", "uuv")
    "Generic" = @()
}

$categorized = @{}
foreach ($cat in $categories.Keys) {
    $categorized[$cat] = @()
}

foreach ($model in $models) {
    $modelName = $model.Name.ToLower()
    $matched = $false
    
    foreach ($cat in $categories.Keys) {
        if ($cat -eq "Generic") { continue }
        
        foreach ($keyword in $categories[$cat]) {
            if ($modelName -like "*$keyword*") {
                $categorized[$cat] += $model
                $matched = $true
                break
            }
        }
        if ($matched) { break }
    }
    
    if (-not $matched) {
        $categorized["Generic"] += $model
    }
}

# Display categorized models
foreach ($cat in $categorized.Keys | Sort-Object) {
    if ($categorized[$cat].Count -gt 0) {
        Write-Host "  [$cat] - $($categorized[$cat].Count) models" -ForegroundColor Yellow
        $categorized[$cat] | Select-Object -First 5 | ForEach-Object {
            $relativePath = $_.FullName.Replace($OutputPath, "").TrimStart("\")
            Write-Host "    • $relativePath" -ForegroundColor Gray
        }
        if ($categorized[$cat].Count -gt 5) {
            Write-Host "    ... and $($categorized[$cat].Count - 5) more" -ForegroundColor DarkGray
        }
        Write-Host ""
    }
}

# ═══════════════════════════════════════════════════════════════
# 3. CREATE UNREAL IMPORT GUIDE
# ═══════════════════════════════════════════════════════════════

Write-Host "─────────────────────────────────────────────────────────────────" -ForegroundColor White
Write-Host "3. CREATING UNREAL IMPORT GUIDE" -ForegroundColor Cyan
Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White

$importGuide = @"
UNREAL ENGINE IMPORT GUIDE - DEFENSE ROBOTICS ASSETS
═══════════════════════════════════════════════════════════════

EXTRACTED MODEL LOCATIONS
══════════════════════════════════════════════════════════════

PX4 Models: $OutputPath\PX4\
  - Quadcopters: iris, typhoon_h480, solo, matrice_100
  - Fixed-wing: plane, standard_vtol, tailsitter
  - VTOL: tiltrotor, quad_tailsitter
  - Ground: rover, ackermann, differential

Gazebo Models: $OutputPath\Gazebo\
  - Various robots and vehicles
  - Props and environment objects
  - Sensor models

RotorS Models: $OutputPath\RotorS\
  - Research-grade MAVs
  - High-fidelity aerodynamics
  - Multiple quadcopter variants

Clover Models: $OutputPath\Clover\
  - Educational quadcopter
  - Complete system including sensors
  - ROS-compatible

═══════════════════════════════════════════════════════════════
IMPORT WORKFLOW FOR EACH MODEL TYPE
═══════════════════════════════════════════════════════════════

COLLADA (.dae) FILES - MOST COMMON
──────────────────────────────────────────

1. Open Unreal Engine 5
2. Content Browser → Import
3. Navigate to .dae file
4. Import Settings:
   ☑ Import Meshes
   ☑ Import Materials
   ☑ Import Textures
   ☐ Import Animations (if any)
   Scale: 100.0 (or adjust based on model size)
   
5. After import:
   - Check mesh scale (should match real vehicle size)
   - Fix materials if needed
   - Add collision meshes

STL FILES - CAD MODELS
──────────────────────────────────────────

1. Convert to FBX first (use Blender):
   - Open Blender
   - File → Import → STL
   - File → Export → FBX
   
2. Import FBX to Unreal (see FBX section)

SDF/URDF/XACRO FILES - ROBOT DESCRIPTION
──────────────────────────────────────────

These are robot definition files (XML format):
- Contain links to mesh files
- Define joint hierarchies
- Specify physics properties

To use:
1. Parse SDF/URDF to find mesh references
2. Import referenced meshes individually
3. Recreate hierarchy in Unreal Blueprint
4. Set up physics constraints

Or use automation tool:
- urdf2gltf: https://github.com/ami-iit/urdf2gltf
- Converts URDF to GLTF, then import to Unreal

═══════════════════════════════════════════════════════════════
RECOMMENDED MODELS FOR UKRAINE SCENARIO
═══════════════════════════════════════════════════════════════

QUADCOPTER RECONNAISSANCE (DJI Mavic equivalent)
• Use: PX4 "iris" or "typhoon_h480"
• Scale: 0.5-0.8 (to match commercial drone size)
• Sensors: RGB camera, optional thermal

LARGE ATTACK DRONE (Bayraktar TB2 equivalent)
• Use: PX4 "plane" or "standard_vtol"
• Scale: 3.0-4.0 (TB2 is 12m wingspan)
• Sensors: Gimbal camera, laser designator
• Note: Will need custom 3D model for accuracy

SMALL FPV DRONE (Racing drone for combat)
• Use: PX4 "solo" (scale down to 0.3-0.4)
• Sensors: FPV camera (first-person view)
• High speed, low altitude

GROUND ROBOT
• Use: PX4 "rover" or Gazebo ground vehicles
• Scale: 1.0-2.0 depending on robot type
• Sensors: Lidar, cameras

═══════════════════════════════════════════════════════════════
CONFIGURING FOR AUTONOMYSIM
═══════════════════════════════════════════════════════════════

After importing to Unreal:

1. CREATE VEHICLE BLUEPRINT
   - Duplicate existing AutonomySim vehicle BP
   - Replace mesh with imported model
   - Adjust collision and physics

2. CONFIGURE SETTINGS.JSON
   
   Example for quadcopter:
   {
     "Vehicles": {
       "ReconDrone1": {
         "VehicleType": "SimpleFlight",
         "DefaultVehicleState": "Armed",
         "X": 0, "Y": 0, "Z": -2,
         "Roll": 0, "Pitch": 0, "Yaw": 0,
         "Cameras": {
           "front_center": {
             "CaptureSettings": [
               {"ImageType": 0, "Width": 1920, "Height": 1080}
             ],
             "X": 0.25, "Y": 0, "Z": 0,
             "Pitch": 0, "Roll": 0, "Yaw": 0
           }
         }
       }
     }
   }

3. ADD TO UNREAL LEVEL
   - Drag vehicle BP into level
   - Or spawn via AutonomySim API

4. TEST WITH PYTHON
   
   import autonomysim as asim
   
   client = asim.MultirotorClient()
   client.confirmConnection()
   client.enableApiControl(True, "ReconDrone1")
   client.armDisarm(True, "ReconDrone1")
   client.takeoffAsync(vehicle_name="ReconDrone1").join()

═══════════════════════════════════════════════════════════════
NEXT STEPS
═══════════════════════════════════════════════════════════════

1. ✓ Models extracted and cataloged
2. ⏳ Import selected models to Unreal
3. ⏳ Create vehicle blueprints
4. ⏳ Configure AutonomySim settings
5. ⏳ Test with Python API
6. ⏳ Create mission scenarios
7. ⏳ Add real-world terrain (Cesium)

For high-fidelity Ukraine scenario:
• Download TB2, Mavic models from Sketchfab/CGTrader
• Set up Cesium terrain of conflict zones
• Use formation control from AutonomySim modules
• Integrate with swarm AI (MCP, A2A, NANDA)

═══════════════════════════════════════════════════════════════
TROUBLESHOOTING
═══════════════════════════════════════════════════════════════

MODEL TOO SMALL/LARGE:
→ Adjust import scale or scale in Blueprint

MISSING TEXTURES:
→ Check texture paths in .dae file
→ Manually assign materials in Unreal

COLLISION ISSUES:
→ Generate collision in Unreal
→ Or create custom collision mesh in Blender

PHYSICS PROBLEMS:
→ Check mass and center of mass
→ Adjust inertia tensor
→ Use AutonomySim physics presets

MODEL DOESN'T FLY CORRECTLY:
→ Verify vehicle type in settings.json
→ Check control surface configuration
→ Tune PID parameters

═══════════════════════════════════════════════════════════════

For more help, see:
• INTEGRATION_GUIDE.txt in parent directory
• AutonomySim documentation: https://microsoft.github.io/AutonomySim
• Unreal Engine docs: https://docs.unrealengine.com

"@

$importGuide | Out-File -FilePath "$OutputPath\IMPORT_GUIDE.txt" -Encoding UTF8
Write-Host "✓ Created: IMPORT_GUIDE.txt`n" -ForegroundColor Green

# ═══════════════════════════════════════════════════════════════
# 4. CREATE MODEL CATALOG CSV
# ═══════════════════════════════════════════════════════════════

Write-Host "─────────────────────────────────────────────────────────────────" -ForegroundColor White
Write-Host "4. CREATING MODEL CATALOG" -ForegroundColor Cyan
Write-Host "─────────────────────────────────────────────────────────────────`n" -ForegroundColor White

$catalog = @()
$catalog += "Category,ModelName,FilePath,FileType,FileSize"

foreach ($cat in $categorized.Keys | Sort-Object) {
    foreach ($model in $categorized[$cat]) {
        $relativePath = $model.FullName.Replace($OutputPath, "").TrimStart("\")
        $sizeKB = [math]::Round($model.Length / 1KB, 2)
        $catalog += "$cat,$($model.BaseName),$relativePath,$($model.Extension),$sizeKB KB"
    }
}

$catalog | Out-File -FilePath "$OutputPath\MODEL_CATALOG.csv" -Encoding UTF8
Write-Host "✓ Created: MODEL_CATALOG.csv ($($models.Count) models cataloged)`n" -ForegroundColor Green

# ═══════════════════════════════════════════════════════════════
# SUMMARY
# ═══════════════════════════════════════════════════════════════

Write-Host "╔═══════════════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║           ASSET EXTRACTION COMPLETE                        ║" -ForegroundColor Green
Write-Host "╚═══════════════════════════════════════════════════════════════╝`n" -ForegroundColor Green

Write-Host "📊 STATISTICS:" -ForegroundColor Cyan
Write-Host "  • Total models found: $($models.Count)" -ForegroundColor White
Write-Host "  • UAV models: $($categorized['UAV'].Count)" -ForegroundColor White
Write-Host "  • UGV models: $($categorized['UGV'].Count)" -ForegroundColor White
Write-Host "  • USV models: $($categorized['USV'].Count)" -ForegroundColor White
Write-Host "  • UUV models: $($categorized['UUV'].Count)" -ForegroundColor White
Write-Host "  • Generic models: $($categorized['Generic'].Count)" -ForegroundColor White

Write-Host "`n📂 OUTPUT LOCATION:" -ForegroundColor Cyan
Write-Host "  $OutputPath" -ForegroundColor White

Write-Host "`n📄 CREATED FILES:" -ForegroundColor Cyan
Write-Host "  ✓ IMPORT_GUIDE.txt - Unreal Engine import instructions" -ForegroundColor White
Write-Host "  ✓ MODEL_CATALOG.csv - Spreadsheet of all models" -ForegroundColor White

Write-Host "`n⚙️  NEXT STEPS:" -ForegroundColor Cyan
Write-Host "  1. Review IMPORT_GUIDE.txt" -ForegroundColor White
Write-Host "  2. Open Unreal Engine 5" -ForegroundColor White
Write-Host "  3. Import selected models (.dae files recommended)" -ForegroundColor White
Write-Host "  4. Create vehicle blueprints" -ForegroundColor White
Write-Host "  5. Configure AutonomySim settings.json" -ForegroundColor White
Write-Host "  6. Test with Python API`n" -ForegroundColor White
