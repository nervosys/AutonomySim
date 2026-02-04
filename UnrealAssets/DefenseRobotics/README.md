# AutonomySim Defense Robotics Assets Collection

## 📋 Overview

This directory contains comprehensive documentation and assets for integrating high-fidelity real-world military robotics into AutonomySim, with a focus on vehicles and environments used in the Ukraine conflict.

## 🎯 What's Included

### ✅ Downloaded Open-Source Assets (Ready to Use)
- **PX4 Gazebo Models** - Quadcopters, fixed-wing, rovers
- **Gazebo Robot Collection** - Various robot models
- **RotorS MAV/UAV Models** - ETH Zurich research drones
- **Clover Educational Drone** - Complete quadcopter system

### 📚 Comprehensive Documentation

| File                                | Description                                                        |
| ----------------------------------- | ------------------------------------------------------------------ |
| `MANUAL_DOWNLOADS_UAV.txt`          | Ukraine conflict drones (TB2, Mavic, Switchblade, Shahed-136, FPV) |
| `MANUAL_DOWNLOADS_UGV.txt`          | Ground robots (Spot, Talon, THeMIS, PackBot)                       |
| `MANUAL_DOWNLOADS_USV_UUV.txt`      | Maritime drones (Sea Baby, Magura V5, Sea Hunter, Bluefin)         |
| `MANUAL_DOWNLOADS_WEAPONS.txt`      | Munitions and weapon systems                                       |
| `MANUAL_DOWNLOADS_ENVIRONMENTS.txt` | Military bases, urban warfare, terrain packs                       |
| `TERRAIN_SETUP_GUIDE.txt`           | Real-world terrain (Cesium, Quixel, OpenStreetMap)                 |
| `INTEGRATION_GUIDE.txt`             | Complete integration workflow                                      |

## 🚁 Featured Ukraine Conflict Drones

### Widely Used UAVs
- **Bayraktar TB2** - Turkish attack drone, high-profile strikes
- **DJI Mavic 3** - Reconnaissance and artillery spotting
- **DJI Phantom 4 Pro** - Modified for munitions drop
- **Switchblade 300/600** - US loitering munitions
- **Shahed-136** - Iranian kamikaze drone (used by Russia)
- **FPV Racing Drones** - Modified commercial drones for combat
- **Phoenix Ghost** - US loitering munition supplied to Ukraine

### NATO/Western Drones
- **MQ-9 Reaper** - US Predator drone
- **RQ-4 Global Hawk** - High-altitude reconnaissance
- **Black Hornet Nano** - Infantry pocket drone

### Russian Drones
- **Orlan-10** - Reconnaissance (frequently intercepted)
- **Lancet** - Loitering munition
- **ZALA** - Tactical reconnaissance

## 🤖 Ground & Maritime Robots

### UGV (Unmanned Ground Vehicles)
- Boston Dynamics Spot (military variant)
- Milrem THeMIS (Estonian armed UGV)
- TALON/PackBot (EOD robots)
- Ripsaw M5 (fast attack)

### USV/UUV (Maritime Drones)
- **Sea Baby** - Ukrainian explosive USV (attacked Russian fleet)
- **Magura V5** - Ukrainian naval drone
- Sea Hunter - DARPA anti-submarine
- Bluefin-21 - Submarine AUV
- REMUS 100/600 - Mine countermeasures

## 🌍 Real-World Terrain Sources

### Method 1: Cesium for Unreal (Recommended)
- **Website**: https://cesium.com/platform/cesium-for-unreal/
- Stream real-world 3D terrain anywhere on Earth
- Photorealistic Google 3D Tiles
- Perfect for Ukraine conflict zones:
  - Crimean Peninsula
  - Donbas region (Donetsk, Luhansk)
  - Mariupol coastal area
  - Kerch Bridge
  - Sevastopol Naval Base

### Method 2: Quixel Megascans (Free)
- **Website**: https://quixel.com/megascans
- Photogrammetry-scanned environments
- Military structures, damaged buildings
- Free with Epic Games account

### Method 3: OpenStreetMap to 3D
- Convert real cities to 3D geometry
- Available for all Ukraine cities
- Road networks and buildings

## 📥 Quick Start

### Step 1: Extract Open-Source Models
```powershell
# Extract downloaded archives
Expand-Archive .\OpenSource\PX4\PX4_Gazebo_Models.zip -DestinationPath .\Models\PX4
Expand-Archive .\OpenSource\Gazebo\Gazebo_Models.zip -DestinationPath .\Models\Gazebo
Expand-Archive .\OpenSource\RotorS\RotorS_UAV_Models.zip -DestinationPath .\Models\RotorS
```

### Step 2: Download High-Fidelity Models
Visit the URLs in the manual download files:
- **Sketchfab** - https://sketchfab.com/search?q=military+drone&type=models
- **GrabCAD** - https://grabcad.com/library?q=military%20drone
- **CGTrader** - https://www.cgtrader.com/3d-models/aircraft/military

### Step 3: Set Up Cesium Terrain
1. Install Cesium for Unreal plugin
2. Get free Cesium Ion token
3. Add "Cesium World Terrain" actor to level
4. Configure camera to Ukraine location

### Step 4: Import to Unreal
```
1. Open AutonomySim Unreal project
2. Content Browser → Import
3. Select downloaded FBX/OBJ files
4. Configure as vehicle pawn
5. Add to settings.json
```

### Step 5: Configure AutonomySim
Edit `~/Documents/AutonomySim/settings.json`:
```json
{
  "SettingsVersion": 1.2,
  "SimMode": "Multirotor",
  "Vehicles": {
    "Bayraktar_TB2": {
      "VehicleType": "SimpleFlight",
      "X": 0, "Y": 0, "Z": -2,
      "Cameras": {
        "front_center": {
          "CaptureSettings": [
            {"ImageType": 0, "Width": 1920, "Height": 1080}
          ]
        }
      }
    }
  }
}
```

### Step 6: Test with Python API
```python
import autonomysim as asim

# Connect to simulator
client = asim.MultirotorClient()
client.confirmConnection()
client.enableApiControl(True)

# Arm and takeoff
client.armDisarm(True)
client.takeoffAsync().join()

# Fly mission
client.moveToPositionAsync(0, 0, -50, 10).join()
```

### Step 7: Swarm Operations
```python
from autonomysim.modules.ai import AgenticSwarmController

# Create swarm
swarm = AgenticSwarmController()
swarm.addAgent("TB2_Leader")
swarm.addAgent("Mavic_Scout_1")
swarm.addAgent("Mavic_Scout_2")

# Set formation
swarm.setFormation(FormationType.Wedge)

# Execute mission
mission_id = swarm.createMission(
    MissionType.SearchAndRescue,
    target_location=(100, 200, -50)
)
swarm.startMission(mission_id)
```

## 🎮 Mission Scenarios

### 1. Reconnaissance Mission
- 3x DJI Mavic scouts
- 1x Bayraktar TB2 overwatch
- Cesium terrain of Donbas region
- AI-coordinated search pattern

### 2. Naval Strike
- Ukrainian Sea Baby USVs
- Target Russian naval vessels
- Black Sea coastal environment
- Swarm coordination attack

### 3. FPV Drone Assault
- Multiple FPV racing drones
- Low-level approach
- Target fortified position
- Manual/autonomous switching

### 4. Multi-Domain Operation
- UAV air superiority
- UGV ground advance
- USV coastal patrol
- Coordinated timing

## 📊 Asset Statistics

| Category     | Asset Count     | Sources                      |
| ------------ | --------------- | ---------------------------- |
| UAV Models   | 50+ documented  | Sketchfab, GrabCAD, CGTrader |
| UGV Models   | 20+ documented  | GitHub, CAD libraries        |
| USV/UUV      | 15+ documented  | Maritime repositories        |
| Environments | 30+ documented  | Unreal Marketplace, Quixel   |
| Weapons      | 25+ documented  | Military model sites         |
| **Total**    | **140+ assets** | **Multiple sources**         |

## 🔗 Key Resource Links

### 3D Model Sources
- **Sketchfab**: https://sketchfab.com (Free & paid)
- **GrabCAD**: https://grabcad.com (Free CAD models)
- **CGTrader**: https://www.cgtrader.com (Free & paid)
- **Free3D**: https://free3d.com (Free models)
- **TurboSquid**: https://www.turbosquid.com (High-quality paid)

### Terrain & Environment
- **Cesium**: https://cesium.com
- **Quixel Megascans**: https://quixel.com/megascans
- **OpenStreetMap**: https://www.openstreetmap.org
- **USGS Earth Explorer**: https://earthexplorer.usgs.gov

### Open-Source Robotics
- **PX4 Autopilot**: https://github.com/PX4
- **Gazebo Models**: https://github.com/osrf/gazebo_models
- **ROS Robots**: https://github.com/ros

### AutonomySim Integration
- **Documentation**: https://microsoft.github.io/AutonomySim
- **Python API**: https://microsoft.github.io/AutonomySim/docs/apis/
- **Settings**: https://microsoft.github.io/AutonomySim/docs/settings/

## 🛠️ Tools & Software Needed

### Essential
- **Unreal Engine 5** - Game engine
- **Blender** - 3D model editing/conversion
- **Python 3.8+** - API scripting

### Recommended Plugins
- **Cesium for Unreal** - Real-world terrain
- **Quixel Bridge** - Asset integration
- **Datasmith** - CAD import

### Optional
- **World Machine** - Terrain generation
- **Substance Painter** - Texture creation
- **RealityCapture** - Photogrammetry

## 💡 Tips & Best Practices

### Model Optimization
- Keep polygon count under 50K for real-time
- Use LOD (Level of Detail) meshes
- Optimize textures to 2K resolution
- Bake lighting where possible

### Performance
- Limit simultaneous vehicles to 10-20
- Use instanced meshes for swarms
- Enable occlusion culling
- Reduce shadow resolution

### Realism
- Match real-world specifications (size, weight, performance)
- Use accurate flight characteristics
- Add sensor noise and limitations
- Include communication delays

### Ukraine Scenario Accuracy
- Research actual tactics and formations
- Use correct vehicle variants
- Match environmental conditions
- Include realistic mission constraints

## 📝 License Notes

- **Open-source models**: Check individual licenses (usually MIT/GPL)
- **Commercial models**: Requires purchase from marketplaces
- **Quixel/Epic content**: Free for Unreal Engine projects
- **Military specifications**: Public domain information only
- **AutonomySim**: MIT License

## 🤝 Contributing

Found more assets or better sources? Please contribute:
1. Document the asset URL and specifications
2. Test integration with AutonomySim
3. Share configuration files
4. Submit pull request or issue

## ⚠️ Disclaimer

This asset collection is for **research, education, and simulation purposes only**. The documentation of military equipment and conflict scenarios is based on publicly available information. Users are responsible for ensuring compliance with export controls, intellectual property rights, and local regulations.

## 📞 Support

- **AutonomySim Issues**: https://github.com/nervosys/AutonomySim/issues
- **Documentation**: See individual .txt files in this directory
- **Community**: AutonomySim Discord/Forums

---

**Last Updated**: October 31, 2025  
**AutonomySim Version**: Latest  
**Total Assets Documented**: 140+  
**Open-Source Downloads**: 4 repositories  
**Documentation Files**: 7 comprehensive guides
