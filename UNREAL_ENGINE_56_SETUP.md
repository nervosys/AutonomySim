# Running AutonomySim in Unreal Engine 5.6

This guide will help you set up and run the AutonomySim robotic swarm visualization in Unreal Engine 5.6.

## Prerequisites

- ✅ **Rust Backend**: Already compiled and ready (`unreal_robotic_swarm.exe`)
- ⚠️ **Unreal Engine 5.6**: Needs to be installed
- ⚠️ **Visual Studio 2022**: Required for UE5 plugin compilation
- ⚠️ **AutonomySim Plugin**: Needs to be built for UE5.6

## Step 1: Install Unreal Engine 5.6

### Option A: Epic Games Launcher (Recommended)
1. Download and install **Epic Games Launcher**: https://www.epicgames.com/store/download
2. In the launcher, go to **Unreal Engine** tab
3. Click **Install Engine** → Select **5.6.x** (latest 5.6 version)
4. Choose installation location (requires ~50GB)
5. Wait for installation to complete

### Option B: Build from Source
```powershell
# Only if you need custom engine modifications
git clone https://github.com/EpicGames/UnrealEngine.git
cd UnrealEngine
git checkout 5.6
.\Setup.bat
.\GenerateProjectFiles.bat
# Open UE5.sln in Visual Studio and build
```

## Step 2: Update AutonomySim Plugin for UE5.6

The plugin needs to be updated to support UE5.6:

```powershell
cd C:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim\UnrealPlugin\Unreal\Plugins\AutonomySim
```

### Update Engine Version in .uplugin

Edit `AutonomySim.uplugin` to add UE5.6 compatibility:

```json
{
  "FileVersion": 3,
  "Version": "1.8.1",
  "VersionName": "1.8.1",
  "FriendlyName": "AutonomySim",
  "Description": "AutonomySim - Autonomous Vehicles Simulator Plugin",
  "Category": "Science",
  "CreatedBy": "Adam Erickson",
  "CreatedByURL": "http://github.com/nervosys/AutonomySim",
  "DocsURL": "https://github.com/nervosys/AutonomySim/blob/master/README.md",
  "SupportURL": "https://github.com/nervosys/AutonomySim/issues",
  "EnabledByDefault": true,
  "CanContainContent": true,
  "IsBetaVersion": true,
  "Installed": true,
  "EngineVersion": "5.6.0",
  "Modules": [
    {
      "Name": "AutonomySim",
      "Type": "Runtime",
      "LoadingPhase": "PreDefault",
      "PlatformAllowList": ["Win64"],
      "TargetAllowList": ["Editor", "Game"]
    }
  ],
  "Plugins": [
    {
      "Name": "ChaosVehiclesPlugin",
      "Enabled": true
    }
  ]
}
```

## Step 3: Create UE5.6 Project

### Quick Method: Create from Template

1. **Launch Unreal Engine 5.6**
2. **Create New Project**:
   - Template: **Blank** or **Third Person**
   - Project Type: **C++** (Important!)
   - Target Platform: **Desktop**
   - Quality Preset: **Maximum Quality**
   - Starter Content: **Yes** (recommended)
   - Project Name: `AutonomySimDemo`
   - Location: `C:\Users\adamm\UE5Projects\AutonomySimDemo`

3. **Close the project** after it's created

### Install AutonomySim Plugin

```powershell
# Create Plugins directory if it doesn't exist
$ProjectDir = "C:\Users\adamm\UE5Projects\AutonomySimDemo"
New-Item -ItemType Directory -Force -Path "$ProjectDir\Plugins"

# Copy AutonomySim plugin
Copy-Item -Recurse `
    "C:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim\UnrealPlugin\Unreal\Plugins\AutonomySim" `
    "$ProjectDir\Plugins\AutonomySim"
```

### Regenerate Project Files

Right-click on `AutonomySimDemo.uproject` → **Generate Visual Studio project files**

### Build the Plugin

1. Open `AutonomySimDemo.sln` in Visual Studio 2022
2. Set configuration to **Development Editor**
3. Build Solution (Ctrl+Shift+B)
4. Wait for compilation (5-10 minutes first time)

## Step 4: Configure AutonomySim in UE5

### Launch the Project

1. Open `AutonomySimDemo.uproject`
2. When prompted about missing modules, click **Yes** to rebuild
3. Wait for Unreal Editor to load

### Enable the Plugin

1. **Edit → Plugins**
2. Search for **AutonomySim**
3. ✅ Check **Enabled**
4. Click **Restart Now**

### Create Urban Environment Level

1. **File → New Level → Empty Level**
2. **Save As**: `Content/Maps/UrbanCity`

3. Add urban environment:
   - **Content Drawer** → Search "City" or "Building"
   - Drag in building assets from starter content
   - Or use **Modeling Tools** to create simple buildings
   - Add ground plane (scale 100x100)
   - Add directional light
   - Add sky atmosphere
   - Add post process volume

### Configure AutonomySim Settings

Create `Config/settings.json` in your project:

```json
{
  "SettingsVersion": 1.2,
  "SimMode": "Multirotor",
  "ClockType": "SteppableClock",
  "RpcEnabled": true,
  "ApiServerPort": 41451,
  "Vehicles": {
    "Drone1": {
      "VehicleType": "SimpleFlight",
      "X": 0, "Y": 0, "Z": 0
    }
  }
}
```

### Add AutonomySim Actor

1. **Window → Place Actors**
2. Search for **AutonomySim** actors
3. Drag `BP_AutonomySimGameMode` into level
4. Or manually:
   - **Blueprints → Open Level Blueprint**
   - Add **AutonomySim Initialize** node to Event BeginPlay

### Enable RPC Server

Add to Level Blueprint (Blueprints → Open Level Blueprint):

```
Event BeginPlay → Enable AutonomySim RPC (Port: 41451)
```

## Step 5: Run the Simulation

### In Unreal Editor:

1. Click **▶ Play** button (or press Alt+P)
2. Wait for "AutonomySim RPC Server Started" message in Output Log
3. Check **Window → Output Log** for:
   ```
   LogAutonomySim: RPC Server listening on port 41451
   ```

### In PowerShell (Rust Backend):

```powershell
cd C:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim\rust

# Run the Unreal Engine example
cargo run --example unreal_robotic_swarm --features unreal --release
```

### Expected Output:

**Unreal Editor**:
- 1,000 robots spawning in the scene
- UAVs flying, UGVs driving
- Real-time rendering @ 60 FPS

**Rust Console**:
```
✓ Unreal Engine 5 connected and ready for rendering
Spawning 1000 robots in Unreal Engine...
✓ All robots spawned in Unreal Engine!

UNREAL ENGINE 5 VISUALIZATION:
  Status:           🎮 RENDERING ACTIVE
  Scene:            Urban City Environment
  Camera Mode:      Follow Fleet
  Rendering:        4K @ 60 FPS
```

## Step 6: Camera Controls in UE5

While simulation is running:

- **F8**: Eject from pawn (free camera)
- **WASD**: Move camera
- **Mouse**: Look around
- **Q/E**: Up/Down
- **Mouse Wheel**: Change speed
- **F**: Focus on selected robot
- **Shift**: Move faster

## Troubleshooting

### Connection Refused (Port 41451)

**Problem**: Rust backend can't connect to UE5
```
Error: No connection could be made (os error 10061)
```

**Solutions**:
1. ✅ Make sure UE5 editor is in **Play** mode
2. ✅ Check Output Log shows "RPC Server listening on port 41451"
3. ✅ Firewall might be blocking - add exception
4. ✅ Try running UE5 as Administrator

### Plugin Won't Load

**Problem**: "Plugin 'AutonomySim' failed to load"

**Solutions**:
1. Verify plugin is in correct location: `[Project]/Plugins/AutonomySim/`
2. Rebuild project: Right-click `.uproject` → Generate Visual Studio files
3. Build in Visual Studio: Development Editor configuration
4. Check UE version matches in `.uplugin` file

### Robots Not Spawning

**Problem**: Connected but no robots appear

**Solutions**:
1. Check vehicle blueprints exist in Content Browser
2. Ensure `BP_AutonomySimGameMode` is placed in level
3. Look for spawn errors in Output Log
4. Verify level has ground plane (robots may fall through world)

### Performance Issues

**Problem**: FPS drops below 30 with 1,000 robots

**Solutions**:
1. **Reduce robot count**: Edit Rust code, change `num_robots` to 100 or 500
2. **Lower graphics quality**: Settings → Engine Scalability Settings → Low
3. **Disable shadows**: Each robot → Details → Lighting → Cast Shadow = false
4. **Use LODs**: Lower detail for distant robots
5. **Enable instancing**: Use Instanced Static Mesh Components

## Quick Start Script

Save this as `run_autonomysim_ue56.ps1`:

```powershell
# Launch Unreal Engine project
Start-Process "C:\Users\adamm\UE5Projects\AutonomySimDemo\AutonomySimDemo.uproject"

# Wait for UE5 to load and enter Play mode
Write-Host "Waiting for Unreal Engine 5 to load..."
Write-Host "Press PLAY in UE5, then press Enter here to connect Rust backend..."
Read-Host

# Launch Rust backend
cd C:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim\rust
cargo run --example unreal_robotic_swarm --features unreal --release
```

Run with:
```powershell
.\run_autonomysim_ue56.ps1
```

## Performance Expectations

With UE5.6 rendering enabled:

| Robots | Resolution | Quality | FPS | Simulation Speed |
| ------ | ---------- | ------- | --- | ---------------- |
| 100    | 1080p      | Epic    | 120 | 10x real-time    |
| 500    | 1080p      | High    | 60  | 5x real-time     |
| 1,000  | 1080p      | Medium  | 60  | 2x real-time     |
| 1,000  | 4K         | Medium  | 30  | 1x real-time     |
| 5,000  | 1080p      | Low     | 30  | 1x real-time     |

**Note**: These are estimates. Actual performance depends on your GPU.

## Next Steps

Once running successfully:

1. **Record footage**: Use UE5's Sequencer to capture video
2. **Add trails**: Show robot movement paths
3. **Visualize RF links**: Draw lines between communicating robots
4. **Show jamming effects**: Red zones where comms are degraded
5. **Custom materials**: Make robots look cooler
6. **Add UI**: Display statistics overlay in viewport

## Resources

- **UE5.6 Documentation**: https://docs.unrealengine.com/5.6/
- **AutonomySim Repo**: https://github.com/nervosys/AutonomySim
- **Discord/Support**: Check repository for community links

---

**Current Status**:
- ✅ Rust backend compiled and ready
- ⚠️ Need to install UE5.6
- ⚠️ Need to build UE5 project with plugin
- ⚠️ Need to create urban environment

**Estimated Setup Time**: 1-2 hours (mostly downloads and compilation)
