# AutonomySim Architecture

> **Date**: February 23, 2026
> **Status**: Active — guides all development decisions

---

## 1. Fundamental Problems with the Current Architecture

### 1.1 Wrong Abstraction Boundary

The `SimulationBackend` trait (17 async methods in `backend.rs`) was designed for a **single-vehicle simulator** — `spawn_vehicle()`, `set_vehicle_control()`, `get_sensor_data()`. But the actual product is a **10,000-agent swarm simulator**. The trait doesn't support batch operations, debug visualization, FPV, telemetry streaming, or any of the 20 RPC methods the system actually uses.

Result: every working example **bypasses the trait** and calls `UnrealConnection` methods directly. The `UnrealEngine5Backend` trait implementation is 15 stubs returning `Ok(())` or `Ok(None)`. The backend trait is dead weight.

### 1.2 Two Conflicting Paradigms

The trait implies a **pull model** (Rust queries UE5: `get_vehicle_state()`, `get_sensor_data()`), but the system actually uses a **push model** (Rust runs physics, pushes state to UE5 for rendering). These are fundamentally different architectures. The system tries to be both and is neither.

**Correct answer**: Rust owns physics. UE5 owns rendering. Push only.

### 1.3 JSON Protocol Bottleneck

10,000 position updates × ~100 bytes/entry = **1 MB of JSON per frame**. At 30 fps = **30 MB/s of JSON**. `FJsonSerializer::Deserialize` on UE5's game thread processes this character by character. This is the #1 performance bottleneck for scaling beyond 2,000 agents.

A binary protocol (MessagePack, FlatBuffers, or raw packed structs) would be **10–50× faster** for serialization/deserialization while using 3–5× less bandwidth.

### 1.4 Synchronous Request/Response for Fire-and-Forget Data

Every `update_positions` call sends a JSON message, **blocks waiting for a response**, then drops the response. Position updates are fire-and-forget — there's nothing useful in the response. The Rust side blocks on every message, serializing all communication through `Mutex<TcpStream>`:

```
send positions → block → read "ok" → unlock → send telemetry → block → read "ok" → unlock
```

This should be:

```
send positions → send telemetry → send debug_lines  (no blocking)
```

### 1.5 Game-Thread Parsing

UE5's `AAutonomySimRPCServer::Tick()` does TCP recv + JSON parse + state update **all on the game thread**. For large payloads, this causes frame hitching. Network I/O and parsing should run on a background thread; only the final state swap should happen on the game thread.

### 1.6 Data Loss in Dual Code Paths

There are **two ways** to send every message:
- `connection.send_message(UnrealMessage::UpdateFpvState{...})` — drops speed, altitude, motors, battery, armed, osd (only sends x/y/z/quat)
- `connection.update_fpv_state(&state)` — sends all fields correctly

The `send_message` path silently loses data. Five message types (`SetControl`, `GetState`, `CastRay`, `CaptureImage`, `RemoveVehicle`) fall through to `_ => ("ping", json!({}))` — they become silent NOPs.

### 1.7 Dead Code

| Component                  | Issue                                              |
| -------------------------- | -------------------------------------------------- |
| `autonomysim-instant-rm`   | Crate is entirely empty (`lib.rs` = whitespace)    |
| `BackendFactory`           | Can't create any non-Native backend (circular dep) |
| `send_message` 5 variants  | Fall through to `ping` NOP                         |
| Summoner workers           | All physics/sensor/comms stubs (`debug!` only)     |
| Isaac/MuJoCo/Warp backends | Structure only, no real code                       |
| FPV PID I/D/F gains        | Declared, configured, serialized, **never used**   |
| Prometheus metrics         | Port declared, server never started                |

### 1.8 Broken Code

| Bug                                                   | Impact                                                                                |
| ----------------------------------------------------- | ------------------------------------------------------------------------------------- |
| `SpatialPartitioner::grid_to_id`: `ix * ny + iy + iz` | ID collisions for 3D grids (should be `ix * ny * nz + iy * nz + iz`)                  |
| `SetVisualizationMode` field mismatch                 | Rust sends 4 booleans; UE5 `HandleSetVisualizationMode` expects a string `mode` field |
| No reconnection logic                                 | UE5 restart = simulation crash                                                        |
| Coordinate convention chaos                           | Core (undefined), FPV (NWU z-up), Tactical (NED z-down), Unreal (z-up, cm)            |

---

## 2. Architectural Principles

### P1: Simulation Owns Physics; Rendering is a Subscriber

Rust is the **source of truth** for all agent state. UE5, CLI, web dashboards — these are **stateless render subscribers** that receive world-state snapshots. They never compute physics or own state.

### P2: Binary Protocol, Not JSON

For high-frequency bulk data (positions, telemetry), use a packed binary format. JSON is reserved for low-frequency setup commands and human debugging.

### P3: Unidirectional Streaming for Bulk Data

Position updates, telemetry, and debug visualization are **fire-and-forget streams**. Only commands (spawn, reset, pause) need request/response semantics.

### P4: Off-Thread Network I/O

Neither Rust's simulation loop nor UE5's game thread should block on network I/O. Both sides use background threads/tasks for serialization and transport.

### P5: One Code Path per Message

Every message type has exactly one serialization function. No dual `send_message` / direct-method paths with different field sets.

### P6: Dead Code is Deleted, Not Maintained

Empty crates, unreachable branches, and stub implementations waste build time and create false confidence. Delete them or implement them.

---

## 3. Target Architecture

```
┌──────────────────────────────────────────────────────────────────┐
│                         RUST PROCESS                              │
│                                                                   │
│  ┌────────────────────────────────────────────────────────────┐  │
│  │                    Simulation Engines                       │  │
│  │                                                            │  │
│  │  ┌──────────┐  ┌──────────┐  ┌─────────┐  ┌───────────┐  │  │
│  │  │ Summoner │  │ RF Core  │  │Tactical │  │ FPV       │  │  │
│  │  │ (swarm)  │  │ (propag) │  │ (EW)    │  │ (physics) │  │  │
│  │  └────┬─────┘  └────┬─────┘  └────┬────┘  └─────┬─────┘  │  │
│  │       └──────────────┴─────────────┴─────────────┘        │  │
│  │                          │                                 │  │
│  │              ┌───────────▼────────────┐                    │  │
│  │              │    WorldState (ECS)    │                    │  │
│  │              │  positions, rotations, │                    │  │
│  │              │  telemetry, RF state   │                    │  │
│  │              └───────────┬────────────┘                    │  │
│  └──────────────────────────┼─────────────────────────────────┘  │
│                             │                                     │
│  ┌──────────────────────────▼─────────────────────────────────┐  │
│  │              Transport Layer (autonomysim-transport)        │  │
│  │                                                            │  │
│  │  ┌──────────────┐  ┌───────────────┐  ┌────────────────┐  │  │
│  │  │ Binary Frame │  │ Async Channel │  │ Connection Mgr │  │  │
│  │  │ Codec        │  │ (tokio mpsc)  │  │ (reconnect)    │  │  │
│  │  └──────────────┘  └───────────────┘  └────────────────┘  │  │
│  │                                                            │  │
│  │  Message types:                                            │  │
│  │  ├── Stream: positions, telemetry, debug  (no response)   │  │
│  │  └── Command: spawn, pause, reset, query  (req/response)  │  │
│  └────────────────────────────────────────────────────────────┘  │
└──────────────────────────┬───────────────────────────────────────┘
                           │  TCP / Shared Memory
┌──────────────────────────▼───────────────────────────────────────┐
│                      UNREAL ENGINE 5.7                            │
│                                                                   │
│  ┌────────────────────────────────────────────────────────────┐  │
│  │            AAutonomySimRPCServer (Actor)                   │  │
│  │                                                            │  │
│  │  ┌─────────────┐  Background Thread:                       │  │
│  │  │ Recv Thread │  - TCP recv loop                          │  │
│  │  │ (FRunnable) │  - Binary decode                          │  │
│  │  └──────┬──────┘  - Write to double buffer                 │  │
│  │         │                                                  │  │
│  │  ┌──────▼──────┐  Game Thread (Tick):                      │  │
│  │  │ State Swap  │  - Swap read/write buffers                │  │
│  │  │ (atomic)    │  - Apply ISM transforms                   │  │
│  │  └──────┬──────┘  - Draw debug shapes                      │  │
│  │         │                                                  │  │
│  │  ┌──────▼──────────────────────────────────────────────┐   │  │
│  │  │ Rendering                                           │   │  │
│  │  │ ├── ISM (instanced meshes per type, 10K+ instances) │   │  │
│  │  │ ├── Debug (lines, spheres via DrawDebugHelpers)      │   │  │
│  │  │ ├── FPV camera (SceneCapture2D + post-process)      │   │  │
│  │  │ └── OSD (UMG widget overlay)                        │   │  │
│  │  └─────────────────────────────────────────────────────┘   │  │
│  └────────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────────┘
```

---

## 4. Crate Restructure

### Current (8 crates, 2 dead)

```
autonomysim-rf (umbrella)
├── autonomysim-core           ← bloated: backend trait + vehicles + sensors + FPV + native backend
├── autonomysim-backends       ← 3 of 4 backends are stubs
├── autonomysim-rf-core        ← solid
├── autonomysim-gaussian-splat ← solid
├── autonomysim-instant-rm     ← EMPTY
├── autonomysim-bindings       ← solid (FFI)
├── autonomysim-tactical       ← solid
└── autonomysim-summoner       ← workers are stubs
```

### Proposed (8 crates, 0 dead)

```
autonomysim-rf (umbrella, re-exports)
├── autonomysim-core           ← SLIMMED: types, traits, coordinate system, NO backend trait
│   ├── types.rs               ← Transform, Position, Vec3, Rotation, Material
│   ├── vehicle.rs             ← VehicleType, VehicleState, VehicleControl, VehicleSpec
│   ├── sensor.rs              ← SensorData, SensorType
│   ├── fpv.rs                 ← FPV physics (unchanged, add I/D to PID)
│   ├── world.rs               ← NEW: WorldState (canonical agent state container)
│   └── coords.rs              ← NEW: coordinate convention + conversions
├── autonomysim-transport      ← RENAMED from autonomysim-backends
│   ├── protocol.rs            ← message types (binary + JSON)
│   ├── codec.rs               ← NEW: binary frame encoder/decoder
│   ├── connection.rs           ← async TCP with reconnection
│   └── unreal.rs              ← UE5-specific send helpers
├── autonomysim-rf-core        ← unchanged
├── autonomysim-gaussian-splat ← unchanged
├── autonomysim-bindings       ← updated for new types
├── autonomysim-tactical       ← deduplicate utils
└── autonomysim-summoner       ← implement worker stubs or mark experimental
```

**Deleted**: `autonomysim-instant-rm` (empty), the broken `BackendFactory`, the `NativeBackend` (relocate to tests or `autonomysim-core::test_utils`).

### Key Changes

1. **Delete `SimulationBackend` trait**. It's the wrong abstraction. Replace with:
   - `WorldState`: a shared state container that simulation engines write to
   - `TransportSink`: a trait for sending world state to renderers

2. **Rename `autonomysim-backends` → `autonomysim-transport`**. It's not a backend — it's a transport layer. It doesn't simulate anything. It moves data.

3. **Add `WorldState` to core**:
   ```rust
   pub struct WorldState {
       pub agents: Vec<AgentState>,        // positions, rotations, types
       pub telemetry: Vec<AgentTelemetry>, // battery, health, signal
       pub fpv_drones: Vec<FpvDroneState>, // FPV-specific state
       pub tick: u64,
       pub time: f64,
       pub paused: bool,
   }
   ```

4. **Add `coords.rs`** with explicit frame definitions:
   ```rust
   pub enum CoordinateFrame { NWU, NED, UnrealEngine }
   pub fn convert(pos: Vec3, from: CoordinateFrame, to: CoordinateFrame) -> Vec3;
   ```

---

## 5. Protocol Redesign

### 5.1 Message Categories

| Category    | Frequency | Semantics        | Format               |
| ----------- | --------- | ---------------- | -------------------- |
| **Stream**  | 30–120 Hz | Fire-and-forget  | Binary (MessagePack) |
| **Command** | < 1 Hz    | Request/response | JSON-RPC 2.0         |
| **Query**   | On-demand | Request/response | JSON-RPC 2.0         |

### 5.2 Stream Messages (Binary, No Response)

```
Frame header: [msg_type: u8] [payload_len: u32]

UpdatePositions (0x01):
  [count: u32]
  [id: i32, x: f32, y: f32, z: f32, qx: f32, qy: f32, qz: f32, qw: f32] × count
  = 32 bytes/agent → 320 KB for 10K agents (vs 1 MB JSON)

UpdateTelemetry (0x02):
  [count: u32]
  [id: i32, battery: f32, health: f32, signal: f32, flags: u8] × count
  = 17 bytes/agent

DebugLines (0x03):
  [count: u32]
  [x1: f32, y1: f32, z1: f32, x2: f32, y2: f32, z2: f32, r: u8, g: u8, b: u8, a: u8, thick: f32] × count

DebugSpheres (0x04):
  [count: u32]
  [x: f32, y: f32, z: f32, radius: f32, r: u8, g: u8, b: u8, a: u8] × count
```

### 5.3 Command Messages (JSON-RPC, Response Required)

```json
{"jsonrpc":"2.0","id":1,"method":"spawn_robots","params":{"robots":[...]}}
{"jsonrpc":"2.0","id":2,"method":"pause","params":{}}
{"jsonrpc":"2.0","id":3,"method":"reset","params":{}}
{"jsonrpc":"2.0","id":4,"method":"get_all_states","params":{}}
```

### 5.4 Backward Compatibility

During migration, support both protocols:
- Binary frames start with a non-`{` byte (the `msg_type` u8)
- JSON messages start with `{`
- The receiver can trivially dispatch on the first byte

---

## 6. UE5 Server Redesign

### 6.1 Background Thread I/O

```cpp
// Current (all game thread):
void Tick() {
    Recv();        // blocks or polls
    ParseJSON();   // expensive for large payloads
    UpdateState(); // fine
    Render();      // fine
}

// Proposed:
// Background FRunnable thread:
void Run() {
    while (bRunning) {
        Recv();
        Decode();  // binary: memcpy; JSON: parse
        WriteToBackBuffer();  // lock-free double buffer
    }
}

// Game thread Tick:
void Tick() {
    SwapBuffers();     // atomic pointer swap, O(1)
    ApplyTransforms(); // ISM batch update
    DrawDebug();
}
```

### 6.2 ISM Batch Updates

Current code updates ISM instances one at a time in a loop. For 10K agents, use `BatchUpdateInstancesTransforms()` (UE5.1+) which updates all transforms in a single GPU upload.

### 6.3 Niagara Path (Future, 100K+ Agents)

For >10K agents, ISM hits CPU limits on transform computation. Niagara particle system with GPU simulation can handle 100K+ point sprites with position data streamed via a compute buffer.

---

## 7. Implementation Priority

### Phase A: Fix Critical Bugs (This Session)

| #   | Fix                                                                     | Impact           |
| --- | ----------------------------------------------------------------------- | ---------------- |
| A1  | Delete `_ => ping` fallthrough in `send_message`; make match exhaustive | Silent data loss |
| A2  | Fix `UpdateFpvState` in `send_message` to include all fields            | FPV data dropped |
| A3  | Fix `SetVisualizationMode` Rust→UE5 field mismatch                      | Vis modes broken |
| A4  | Fix `SpatialPartitioner::grid_to_id` for 3D grids                       | ID collisions    |
| A5  | Delete `autonomysim-instant-rm` (empty crate)                           | Build time waste |
| A6  | Delete `BackendFactory` (broken by design)                              | False API        |
| A7  | Unify `dbm_to_watts`/`watts_to_dbm` (deduplicate tactical↔rf-core)      | Code duplication |

### Phase B: Transport Layer (Next Session)

| #   | Change                                                  | Impact                                |
| --- | ------------------------------------------------------- | ------------------------------------- |
| B1  | Add binary frame codec for positions/telemetry          | 3–5× less bandwidth, 10× faster parse |
| B2  | Make stream messages fire-and-forget (no response)      | 2× throughput                         |
| B3  | Add background recv thread in UE5                       | Eliminate game-thread stutter         |
| B4  | Add reconnection logic to `UnrealConnection`            | Crash resilience                      |
| B5  | Rename `autonomysim-backends` → `autonomysim-transport` | Clarity                               |

### Phase C: Architecture Cleanup (Future)

| #   | Change                                                                | Impact              |
| --- | --------------------------------------------------------------------- | ------------------- |
| C1  | Replace `SimulationBackend` trait with `WorldState` + `TransportSink` | Correct abstraction |
| C2  | Add `coords.rs` with frame definitions and conversions                | Correctness         |
| C3  | Implement FPV PID I/D/F terms                                         | Flight quality      |
| C4  | Implement Summoner worker physics (or mark experimental)              | Honesty             |
| C5  | ISM batch transforms in UE5                                           | ISM performance     |
| C6  | Niagara GPU particles for 100K+                                       | Scale ceiling       |

---

## 8. Performance Targets

| Scale          | Protocol            | UE5 Renderer     | Target FPS | Latency |
| -------------- | ------------------- | ---------------- | ---------- | ------- |
| 1,000 agents   | JSON-RPC (current)  | ISM per-type     | 60 fps     | <5ms    |
| 5,000 agents   | Binary stream       | ISM batch update | 60 fps     | <10ms   |
| 10,000 agents  | Binary stream       | ISM batch + LOD  | 30 fps     | <20ms   |
| 50,000 agents  | Binary stream       | Niagara GPU      | 30 fps     | <50ms   |
| 100,000 agents | Binary stream + shm | Niagara GPU      | 30 fps     | <100ms  |

---

## 9. Test Strategy

| Layer      | What to Test                                   | How                                 |
| ---------- | ---------------------------------------------- | ----------------------------------- |
| Protocol   | Round-trip encode/decode for all message types | Unit tests with proptest            |
| Transport  | Connection, reconnection, partial reads        | Integration tests with mock server  |
| WorldState | Concurrent read/write, state consistency       | Unit + stress tests                 |
| UE5 Server | Binary decode, ISM update correctness          | Automated PIE test (UE5 Automation) |
| End-to-End | Rust → UE5 → verify rendered positions         | CI with headless UE5                |

---

## 10. Non-Goals

- **Physics in UE5**: UE5 is a render-only subscriber. No `PhysX` integration.
- **Web frontend**: Not in scope. CLI + UE5 are the two frontends.
- **ROS2 bridge**: The `ros2/` directory exists but is out of scope for this architecture.
- **Real hardware**: This is simulation only. Hardware integration is a separate project.
