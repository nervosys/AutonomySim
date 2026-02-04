# AutonomySim UE5.7 Integration Roadmap

> **Last Updated**: January 26, 2026  
> **Goal**: Run 1,000-robot swarm simulation with real-time 3D visualization in Unreal Engine 5.7

---

## Project Overview

Integrating the Rust-based AutonomySim robotic swarm simulation backend with Unreal Engine 5.7 for real-time 3D visualization. The system uses RPC communication on port 41451 between Rust and UE5.

### Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                       AutonomySim Architecture                       │
├─────────────────────────────────────────────────────────────────────┤
│                                                                       │
│  ┌─────────────────────┐          ┌─────────────────────────────┐   │
│  │   RUST BACKEND      │          │   UNREAL ENGINE 5.7         │   │
│  │                     │          │                             │   │
│  │  ┌───────────────┐  │  JSON    │  ┌─────────────────────┐   │   │
│  │  │ SUMMONER      │  │  RPC     │  │ AutonomySimRPCServer│   │   │
│  │  │ (4.7M agents/s)│ ◄─────────►  │ (Actor)             │   │   │
│  │  └───────────────┘  │  TCP     │  └─────────────────────┘   │   │
│  │                     │  41451   │           │                │   │
│  │  ┌───────────────┐  │          │  ┌────────▼────────────┐   │   │
│  │  │ RF Propagation│  │          │  │ Instanced Meshes    │   │   │
│  │  └───────────────┘  │          │  │ (1000+ robots)      │   │   │
│  │                     │          │  └─────────────────────┘   │   │
│  │  ┌───────────────┐  │          │                            │   │
│  │  │ Tactical/EW   │  │          │  ┌─────────────────────┐   │   │
│  │  └───────────────┘  │          │  │ Debug Visualization │   │   │
│  │                     │          │  │ (links, RF, paths)  │   │   │
│  └─────────────────────┘          │  └─────────────────────┘   │   │
│                                    │                            │   │
│                                    └─────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
```

### Key Paths

| Component          | Location                                                            |
| ------------------ | ------------------------------------------------------------------- |
| Rust Workspace     | `C:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim\rust`  |
| UE5.7 Installation | `C:\Program Files\Epic Games\UE_5.7`                                |
| UE5 Project        | `C:\Users\adamm\UE5Projects\AutonomySimDemo`                        |
| RPC Plugin         | `C:\Users\adamm\UE5Projects\AutonomySimDemo\Plugins\AutonomySimRPC` |
| Main UE Plugin     | `AutonomySim\UnrealPlugin\Unreal\Plugins\AutonomySim`               |

---

## Phase 1: Rust Backend ✅ COMPLETE

- [x] Build Rust workspace with all crates
- [x] Create `robotic_swarm_demo.rs` (headless demo)
- [x] Run headless simulation successfully
  - **Result**: 4.7M agents/second, 105x real-time performance
  - **Fleet**: 1,000 robots (400 Scout, 300 Transport, 200 Combat, 80 Relay, 20 Coordinator)
- [x] Create `unreal_robotic_swarm.rs` (UE5 visualization demo)
- [x] Compile unreal_robotic_swarm with `--features unreal`

---

## Phase 2: UE5 Project Setup ✅ COMPLETE

- [x] Verify UE5.6 installation
- [x] Create C++ UE5 project "AutonomySimDemo"
- [x] Create minimal `AutonomySimRPC` plugin (original plugin had UE5.6 compatibility issues)
- [x] Build UE5 project with plugin successfully

### Plugin Files Created

```bash
Plugins/AutonomySimRPC/
├── Source/AutonomySimRPC/
│   ├── AutonomySimRPC.Build.cs
│   ├── Private/
│   │   └── AutonomySimRPCServer.cpp
│   └── Public/
│       └── AutonomySimRPCServer.h
└── AutonomySimRPC.uplugin
```

---

## Phase 3: Enhanced RPC Protocol ✅ COMPLETE

- [x] Design comprehensive JSON-RPC protocol
- [x] Implement batched position updates for high performance
- [x] Add robot type differentiation (Scout, Transport, Combat, Relay, Coordinator)
- [x] Add telemetry streaming (battery, health, signal strength, jam status)
- [x] Add debug visualization messages (lines, spheres)
- [x] Add visualization mode control

### RPC Protocol Methods

| Method                       | Description                      | Direction  |
| ---------------------------- | -------------------------------- | ---------- |
| `spawn_robots`               | Batch spawn with type info       | Rust → UE5 |
| `update_positions`           | Batch position/rotation updates  | Rust → UE5 |
| `update_telemetry`           | Battery, health, signal data     | Rust → UE5 |
| `draw_debug_lines`           | Communication link visualization | Rust → UE5 |
| `draw_debug_spheres`         | RF range visualization           | Rust → UE5 |
| `set_visualization_mode`     | Toggle debug overlays            | Rust → UE5 |
| `clear_all_robots`           | Reset visualization              | Rust → UE5 |
| `pause` / `resume` / `reset` | Simulation control               | Rust → UE5 |

### Robot Types & Colors

| Type        | Role               | Color (RGB)           | Altitude |
| ----------- | ------------------ | --------------------- | -------- |
| Scout       | Reconnaissance UAV | Red (255, 64, 64)     | 5-11m    |
| Transport   | Ground cargo       | Blue (64, 128, 255)   | Ground   |
| Combat      | Armed UAV/UGV      | Green (64, 255, 64)   | 3-7m     |
| Relay       | Comm relay node    | Yellow (255, 255, 64) | 8-18m    |
| Coordinator | Command & control  | Purple (192, 64, 255) | 10m      |

---

## Phase 4: UE5 Visualization Actor ✅ COMPLETE

- [x] Create `AAutonomySimRPCServer` actor class
- [x] Implement TCP server on port 41451
- [x] Multi-threaded network handling (game thread safe)
- [x] Instanced static mesh rendering for each robot type
- [x] Blueprint events for extensibility
- [x] Debug visualization (lines, spheres)
- [x] Configurable visualization modes

### Actor Features

```cpp
// Configuration
UPROPERTY(EditAnywhere) int32 ListenPort = 41451;
UPROPERTY(EditAnywhere) bool bAutoStartServer = true;
UPROPERTY(EditAnywhere) int32 MaxRobots = 2000;

// Visualization Settings
UPROPERTY(EditAnywhere) bool bShowCommLinks = true;
UPROPERTY(EditAnywhere) bool bShowRFRange = false;
UPROPERTY(EditAnywhere) bool bShowPaths = false;
UPROPERTY(EditAnywhere) bool bShowLabels = false;

// Robot Type Colors (configurable in Editor)
UPROPERTY(EditAnywhere) FLinearColor ScoutColor;
UPROPERTY(EditAnywhere) FLinearColor TransportColor;
// ... etc

// Blueprint Events
OnRobotSpawned(int32 RobotId, EAutonomySimRobotType Type, FVector Position)
OnPositionsUpdated(int32 Count)
OnClientConnected()
OnClientDisconnected()
OnSimulationStateChanged(bool bIsPaused)
```

---

## Phase 5: UE5 Configuration 🔄 IN PROGRESS

- [ ] Open UE5 project in editor
- [ ] Add `AutonomySimRPCServer` actor to level
- [ ] Assign drone and ground vehicle meshes
- [ ] Create material instances with robot type colors
- [ ] Test connection with Rust backend

### Quick Setup Steps

1. Open UE5 project
2. Place `AutonomySimRPCServer` actor in level
3. Configure meshes:
   - `DroneMesh`: Use built-in Sphere or import custom drone mesh
   - `GroundVehicleMesh`: Use built-in Cube or import custom UGV mesh
4. Set colors in Details panel
5. Press Play

---

## Phase 6: Integration Testing ⏳ PENDING

- [ ] Press Play in UE5 Editor
- [ ] Run Rust backend: `cargo run --example unreal_robotic_swarm --features unreal --release`
- [ ] Verify connection between Rust and UE5
- [ ] Debug any communication issues
- [ ] Test all RPC methods

---

## Phase 7: Performance Optimization ⏳ PENDING

- [ ] Profile RPC communication overhead
- [ ] Implement Niagara particle system for 10K+ robots
- [ ] Add LOD (Level of Detail) for distant robots
- [ ] Implement hierarchical instancing
- [ ] Target: 1,000+ robots at 60fps

---

## Quick Commands

### Build Rust Backend
```powershell
cd C:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim\rust
cargo build --release --features unreal
```

### Run Headless Demo
```powershell
cargo run --example robotic_swarm_demo --release
```

### Run UE5 Visualization Demo
```powershell
cargo run --example unreal_robotic_swarm --features unreal --release
```

### Open UE5 Project
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\adamm\UE5Projects\AutonomySimDemo\AutonomySimDemo.uproject"
```

---

## Notes

### RPC Communication
- **Port**: 41451
- **Protocol**: JSON-RPC 2.0 over TCP (newline-delimited)
- **Message Types**: Spawn, Position Update, Telemetry, Debug Visualization

### Known Issues
1. Original AutonomySim UE plugin has header compatibility issues with UE5.7 (stricter `#include` requirements)
2. Live Coding in UE5 editor blocks command-line builds - close editor before rebuilding

### Performance Benchmarks (Headless)
| Metric          | Value        |
| --------------- | ------------ |
| Agents/Second   | 4,716,981    |
| Steps Completed | 3,000        |
| Total Time      | 0.57 seconds |
| Real-time Ratio | 105x         |

---

## File Locations

### Rust Backend (Enhanced Protocol)
- `rust/autonomysim-backends/src/unreal/protocol.rs` - Message types
- `rust/autonomysim-backends/src/unreal/connection.rs` - TCP connection
- `rust/autonomysim-backends/src/unreal/mod.rs` - Backend interface

### UE5 Actor
- `UnrealPlugin/Unreal/Plugins/AutonomySim/Source/RPC/AutonomySimRPCServer.h`
- `UnrealPlugin/Unreal/Plugins/AutonomySim/Source/RPC/AutonomySimRPCServer.cpp`

---

## Change Log

### 2026-01-26 (Session 2)
- Enhanced RPC protocol with comprehensive message types
- Added `RobotType` enum with color coding (Scout=red, Transport=blue, etc.)
- Added `RobotSpawnData`, `RobotPositionUpdate`, `RobotTelemetry` structs
- Added debug visualization messages (lines, spheres)
- Created `AAutonomySimRPCServer` UE5 actor class
- Implemented multi-threaded TCP server in UE5
- Added instanced mesh rendering for each robot type
- Added Blueprint events for extensibility
- Updated ROADMAP with architecture diagram

### 2026-01-26 (Session 1)
- Updated from UE5.6 to UE5.7 (UE5.6 not installed, UE5.7 is available)
- Built UE5.7 project successfully with AutonomySimRPC plugin
- Fixed header/implementation mismatch in RPC server

### 2026-01-19
- Created AutonomySimRPC minimal plugin for UE5 compatibility
- Successfully built UE5 project with plugin
- Created this roadmap document

### Previous Sessions
- Built Rust workspace and all examples
- Achieved 4.7M agents/second in headless mode
- Fixed compilation errors in unreal_robotic_swarm example
