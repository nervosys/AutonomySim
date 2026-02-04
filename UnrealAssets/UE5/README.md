# AutonomySim - Unreal Engine 5 Assets Guide

**Optimized for UE 5.3 & 5.4 | Complete Setup in 30 Minutes**

## 🎯 Overview

This guide provides **everything you need** to create photorealistic military simulations in Unreal Engine 5 with AutonomySim, featuring:
- **Real-world terrain** (anywhere on Earth via Cesium)
- **16+ free military environments** (UE Marketplace)
- **15,000+ free photoscanned assets** (Quixel Megascans)
- **Ukraine conflict drones** (Bayraktar TB2, Mavic, FPV drones)
- **UE5 features** (Nanite, Lumen, World Partition)

## 📦 What's Included

| File                           | Description                            | Priority          |
| ------------------------------ | -------------------------------------- | ----------------- |
| **QUICK_START_GUIDE.txt**      | 30-minute complete setup workflow      | ⭐⭐⭐⭐⭐ START HERE  |
| **CESIUM_UE5_SETUP_GUIDE.txt** | Real-world terrain (Ukraine locations) | ⭐⭐⭐⭐⭐ RECOMMENDED |
| **UE5_FREE_ENVIRONMENTS.txt**  | 16+ free marketplace environments      | ⭐⭐⭐⭐☆             |
| **UE5_VEHICLE_MODELS.txt**     | Military drone download sources        | ⭐⭐⭐⭐☆             |

## 🚀 Quick Start (30 Minutes)

### Phase 1: Real-World Terrain (10 min) ⭐ RECOMMENDED

**Cesium for Unreal** - Stream Earth anywhere:

1. **Install Plugin**
   - UE5 → Edit → Plugins → Search "Cesium" → Install → Restart

2. **Get Free Token**
   - Sign up: https://cesium.com/ion/signup
   - Copy token from: https://cesium.com/ion/tokens

3. **Add Terrain**
   - Cesium panel → "Cesium World Terrain + Bing Maps"
   - Paste token → Terrain loads automatically

4. **Navigate to Ukraine**
   ```
   Donbas Region: Lat 48.0°N, Lon 37.8°E
   Crimea: Lat 45.0°N, Lon 34.0°E
   Mariupol: Lat 47.0971°N, Lon 37.5431°E
   Kerch Bridge: Lat 45.3°N, Lon 36.5°E
   ```

**Result**: Fly drones over photorealistic Ukraine terrain in 10 minutes! ✅

### Phase 2: Free Assets (10 min)

**Quixel Megascans** - 15,000+ free photoscanned assets:

1. Install Quixel Bridge plugin (built into UE5)
2. Login with Epic Games account
3. Search: "military", "industrial", "destroyed"
4. Drag & drop into scene
5. Automatic Nanite conversion (billions of polygons)

**Popular Categories**:
- Military structures
- Concrete barriers
- Damaged buildings
- Eastern European architecture
- Natural terrain

### Phase 3: Vehicle Models (10 min)

**Download Ukraine Conflict Drones**:

#### Best Free Sources:

1. **Sketchfab** (Best for Military Drones)
   - Bayraktar TB2: https://sketchfab.com/search?q=bayraktar+tb2
   - DJI Mavic 3: https://sketchfab.com/search?q=dji+mavic+3
   - FPV Drones: https://sketchfab.com/search?q=fpv+drone
   - **Format**: FBX, GLTF (UE5-ready)

2. **GrabCAD** (Free CAD Models)
   - TB2: https://grabcad.com/library/bayraktar-tb2
   - Quadcopters: https://grabcad.com/library?q=military%20quadcopter
   - **Format**: STEP (needs Datasmith conversion)

3. **Already Downloaded** (PX4 Models)
   - Location: `..\DefenseRobotics\Models\PX4\`
   - 65+ UAV models ready to import

#### Import to UE5:

```
1. Content Browser → Import
2. Select FBX file
3. ☑ Import Meshes, Materials, Textures
4. Check scale (Mavic ~0.35m, TB2 ~12m)
5. Enable Nanite for high-poly meshes
6. Add collision mesh
```

## 🌟 UE5 Features Utilized

### Nanite Virtualized Geometry
- **Billions of polygons** without performance loss
- Film-quality assets in real-time
- Automatic LOD (Level of Detail)
- Enable on any static mesh

### Lumen Global Illumination
- **Real-time lighting** (no baking required)
- Realistic shadows and reflections
- Dynamic time-of-day
- Perfect for outdoor drone missions

### World Partition
- **Massive open worlds** (100km²+)
- Automatic streaming
- Multiple users editing simultaneously
- Essential for large-scale terrain

### Virtual Shadow Maps
- High-quality shadows
- Consistent across distances
- Lower memory usage

## 🎮 Recommended UE5 Environments

### Free Marketplace Assets (16+ Environments)

| Environment              | Size | UE5 | Use Case                   | Link                                                                                         |
| ------------------------ | ---- | --- | -------------------------- | -------------------------------------------------------------------------------------------- |
| **City of Brass**        | 8 GB | ✅   | Urban warfare, Middle East | [Download](https://www.unrealengine.com/marketplace/en-US/product/city-of-brass)             |
| **Modular Neighborhood** | 3 GB | ✅   | Suburban recon missions    | [Download](https://www.unrealengine.com/marketplace/en-US/product/modular-neighborhood-pack) |
| **Military Base**        | 6 GB | ✅   | Airbase operations         | [Download](https://www.unrealengine.com/marketplace/en-US/product/military-base-environment) |
| **Abandoned Factory**    | 5 GB | ✅   | Industrial UGV missions    | [Download](https://www.unrealengine.com/marketplace/en-US/product/abandoned-factory)         |
| **Destroyed City**       | 7 GB | ✅   | War-torn Ukraine-style     | [Download](https://www.unrealengine.com/marketplace/en-US/product/destroyed-city)            |
| **Realistic Forest**     | 7 GB | ✅   | Recon in tree cover        | [Download](https://www.unrealengine.com/marketplace/en-US/product/realistic-forest-pack)     |

**All FREE** - Requires Epic Games account

### Monthly Free Assets
Check monthly: https://www.unrealengine.com/marketplace/en-US/assets
- Valued at $50-200
- Become permanently free
- Check 1st of each month

## 🚁 Ukraine Conflict Drones

### High-Priority Models

#### Bayraktar TB2 (Turkey)
- **Role**: Attack drone, high-profile strikes
- **Downloads**: 
  - Sketchfab (free): https://sketchfab.com/3d-models/bayraktar-tb2
  - CGTrader ($49): https://www.cgtrader.com/3d-models/aircraft/military/bayraktar-tb2
- **Specs**: 12m wingspan, 1450kg MTOW
- **UE5 Import**: FBX format, scale 1.0

#### DJI Mavic 3 (China)
- **Role**: Reconnaissance, artillery spotting
- **Downloads**:
  - Sketchfab (free): https://sketchfab.com/3d-models/dji-mavic-3
  - TurboSquid ($79): https://www.turbosquid.com/3d-models/dji-mavic-3
- **Specs**: 0.35m wingspan, 0.9kg
- **UE5 Import**: FBX/GLTF, scale 0.01 (if too big)

#### FPV Racing Drone (Modified for Combat)
- **Role**: Close-range strikes, munitions drop
- **Downloads**:
  - Sketchfab (free): https://sketchfab.com/3d-models/fpv-racing-drone
  - CGTrader ($29 for 5): https://www.cgtrader.com/3d-models/vehicle/other/fpv-drone-pack
- **Specs**: 0.25m, ~0.5kg
- **UE5 Import**: High detail, enable Nanite

#### Switchblade 300/600 (US Loitering Munition)
- **Role**: Kamikaze drone, anti-tank
- **Downloads**:
  - CGTrader ($39): https://www.cgtrader.com/3d-models/aircraft/military/switchblade-300
- **Specs**: 0.6m/1.3m length
- **UE5 Import**: May need custom materials

#### Shahed-136 (Iranian Kamikaze - Used by Russia)
- **Role**: Long-range attack
- **Downloads**:
  - Sketchfab (free): https://sketchfab.com/3d-models/shahed-136
- **Specs**: 2.5m wingspan
- **UE5 Import**: Check orientation

### Full List
See: **UE5_VEHICLE_MODELS.txt** for 50+ drone models

## 🔧 AutonomySim Integration

### Configure settings.json

```json
{
  "SettingsVersion": 1.2,
  "SimMode": "Multirotor",
  "ClockSpeed": 1.0,
  "Vehicles": {
    "TB2_Leader": {
      "VehicleType": "SimpleFlight",
      "X": 0, "Y": 0, "Z": -2,
      "Cameras": {
        "front_center": {
          "CaptureSettings": [
            {"ImageType": 0, "Width": 1920, "Height": 1080},
            {"ImageType": 1, "Width": 1920, "Height": 1080}
          ],
          "X": 0.5, "Y": 0, "Z": -0.2
        },
        "gimbal": {
          "CaptureSettings": [
            {"ImageType": 0, "Width": 1920, "Height": 1080}
          ],
          "X": 0, "Y": 0, "Z": -0.5,
          "Pitch": -45
        }
      }
    },
    "Mavic_Scout1": {
      "VehicleType": "SimpleFlight",
      "X": -5, "Y": -5, "Z": -2
    },
    "Mavic_Scout2": {
      "VehicleType": "SimpleFlight",
      "X": 5, "Y": -5, "Z": -2
    }
  }
}
```

### Python Test Flight

```python
import autonomysim as asim

# Connect to simulator
client = asim.MultirotorClient()
client.confirmConnection()
print("✓ Connected to AutonomySim over UE5")

# Arm and takeoff
client.enableApiControl(True, "TB2_Leader")
client.armDisarm(True, "TB2_Leader")
client.takeoffAsync(vehicle_name="TB2_Leader").join()

# Fly reconnaissance pattern over Donbas
waypoints = [
    (0, 0, -100),      # 100m altitude
    (1000, 0, -100),   # 1km east
    (1000, 1000, -100),# 1km north-east  
    (0, 1000, -100),   # 1km north
]

for x, y, z in waypoints:
    client.moveToPositionAsync(x, y, z, 20, vehicle_name="TB2_Leader").join()
    
    # Capture imagery
    responses = client.simGetImages([
        asim.ImageRequest("front_center", asim.ImageType.Scene),
        asim.ImageRequest("gimbal", asim.ImageType.Scene)
    ], vehicle_name="TB2_Leader")
    
    asim.write_file(f'recon_{x}_{y}.png', responses[0].image_data_uint8)

print("✓ Reconnaissance mission complete")
```

### Swarm Coordination (MCP/A2A/NANDA)

```python
from autonomysim.modules.ai import AgenticSwarmController

# Create swarm
swarm = AgenticSwarmController()

# Add agents
swarm.addAgent("TB2_Leader", capabilities={"attack": 0.9, "surveillance": 0.95})
swarm.addAgent("Mavic_Scout1", capabilities={"sensing": 0.95, "recon": 0.9})
swarm.addAgent("Mavic_Scout2", capabilities={"sensing": 0.95, "recon": 0.9})

# Set wedge formation
swarm.setFormation(FormationType.Wedge)
swarm.setFormationLeader("TB2_Leader")

# Create search & rescue mission
mission_id = swarm.createMission(
    MissionType.SearchAndRescue,
    description="Search destroyed city sector",
    target_location=(2000, 1500, -50)
)

# Enable collective intelligence
swarm.enableCollectiveDecisionMaking(True)
swarm.enableEmergentBehaviors(True)

# Execute mission
swarm.startMission(mission_id)

# Monitor progress
while swarm.getMission(mission_id).state == SwarmState.Executing:
    # Get swarm metrics
    cohesion = swarm.getSwarmCohesion()
    centroid = swarm.getSwarmCentroid()
    print(f"Cohesion: {cohesion:.2f}, Center: {centroid}")
    time.sleep(1)
```

## 📊 Performance Optimization

### Hardware Requirements

| Quality         | GPU      | RAM   | Storage   | FPS (3 drones) |
| --------------- | -------- | ----- | --------- | -------------- |
| **Minimum**     | RTX 3060 | 16 GB | SSD       | 30-45          |
| **Recommended** | RTX 4070 | 32 GB | NVMe SSD  | 60-90          |
| **Optimal**     | RTX 4090 | 64 GB | NVMe RAID | 120+           |

### UE5 Settings for Performance

#### Enable for Quality:
- ✅ Nanite (static meshes)
- ✅ Lumen (global illumination)
- ✅ Virtual Shadow Maps
- ✅ Temporal Super Resolution (TSR)

#### Disable if Slow:
- ❌ Lumen (use baked lighting)
- ❌ Reduce Cesium tile detail
- ❌ Lower screen resolution (1080p → 720p)
- ❌ Reduce shadow quality

### Cesium Optimization

```
Cesium Actor → Details:
- Maximum Screen Space Error: 16 (default 2)
- Maximum Cached Bytes: 256 MB (default 512 MB)
- Preload Ancestors: Disabled (enable for smoother)
- Suspend Update: True (when not moving)
```

## 🗺️ Cesium Locations: Ukraine Conflict Zones

### Strategic Locations

| Location           | Coordinates          | Description                | Use Case            |
| ------------------ | -------------------- | -------------------------- | ------------------- |
| **Donbas Region**  | 48.0°N, 37.8°E       | Urban warfare zones        | UAV reconnaissance  |
| **Mariupol**       | 47.0971°N, 37.5431°E | Coastal city, major battle | Search & rescue     |
| **Crimean Bridge** | 45.3°N, 36.5°E       | Strategic bridge           | Strike mission      |
| **Sevastopol**     | 44.6169°N, 33.5254°E | Naval base                 | USV operations      |
| **Kyiv**           | 50.4501°N, 30.5234°E | Capital city               | Air defense         |
| **Kharkiv**        | 49.9935°N, 36.2304°E | Near Russian border        | Border patrol       |
| **Dnipro**         | 48.4647°N, 35.0462°E | River city                 | Bridge surveillance |
| **Kherson**        | 46.6354°N, 32.6169°E | Antonovsky Bridge          | River crossing      |

### Terrain Types Available

- ✅ **Urban** - Destroyed buildings, streets, infrastructure
- ✅ **Agricultural** - Open fields, farm roads
- ✅ **Forest** - Tree cover, concealment
- ✅ **Coastal** - Black Sea, ports, naval facilities
- ✅ **Industrial** - Factories, refineries, power plants
- ✅ **Suburban** - Residential areas, outskirts

## 📚 Additional Resources

### Documentation
- **Quick Start**: QUICK_START_GUIDE.txt (30-min setup)
- **Cesium Setup**: CESIUM_UE5_SETUP_GUIDE.txt (detailed)
- **Environments**: UE5_FREE_ENVIRONMENTS.txt (16+ free assets)
- **Vehicles**: UE5_VEHICLE_MODELS.txt (50+ drone models)

### External Links
- **Cesium**: https://cesium.com/platform/cesium-for-unreal/
- **Quixel**: https://quixel.com/megascans
- **UE Marketplace**: https://www.unrealengine.com/marketplace/
- **Sketchfab**: https://sketchfab.com/search?q=military+drone
- **AutonomySim Docs**: https://microsoft.github.io/AutonomySim/

## 💡 Pro Tips

### Cesium + Quixel Workflow
1. Load Cesium terrain (real-world location)
2. Add Quixel props (military structures, damaged buildings)
3. Blend seamlessly for photorealistic environments
4. Total cost: **FREE** (Epic account required)

### Model Quality Tiers
- **Testing**: Use PX4 models (already downloaded, instant)
- **Demo**: Download free Sketchfab models (good quality)
- **Production**: Purchase premium CGTrader models ($50-200)

### Mission Scenarios
1. **Recon Flight** - Mavic scouts survey area
2. **Strike Mission** - TB2 loitering, precision strike
3. **Swarm Search** - Multiple FPV drones, coordinated pattern
4. **Naval Ops** - Sea Baby USVs, coastal patrol
5. **Multi-Domain** - UAV + UGV + USV coordinated attack

## ⚠️ License & Usage

- **UE5**: Free for projects under $1M revenue
- **Cesium**: Free tier (5M tiles/month), commercial tier available
- **Quixel**: Free for UE projects only
- **Marketplace Assets**: Check individual licenses
- **3D Models**: Free/paid, check usage rights
- **AutonomySim**: MIT License

Educational and research use encouraged. Respect export controls and regulations.

## 🤝 Contributing

Found better assets or workflows? Please contribute:
1. Document asset URL and specifications
2. Test with AutonomySim + UE5
3. Share configuration files
4. Submit pull request

## 📞 Support

- **AutonomySim Issues**: https://github.com/nervosys/AutonomySim/issues
- **UE5 Forums**: https://forums.unrealengine.com/
- **Cesium Support**: https://community.cesium.com/

---

**Last Updated**: October 31, 2025  
**Target Engine**: Unreal Engine 5.3 / 5.4  
**Total Free Assets**: 16+ environments, 15,000+ props, 50+ vehicles  
**Setup Time**: 30 minutes  
**Cost**: FREE (Epic Games account required)

**Ready to simulate photorealistic Ukraine conflict scenarios with UE5 + AutonomySim!** 🚁🌍
