// AutonomySimRPCServer.cpp
// JSON-RPC server implementation for AutonomySim

#include "RPC/AutonomySimRPCServer.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"

// Log category
DEFINE_LOG_CATEGORY_STATIC(LogAutonomySimRPC, Log, All);

//=============================================================================
// AAutonomySimRPCServer Implementation
//=============================================================================

AAutonomySimRPCServer::AAutonomySimRPCServer() {
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.TickInterval = 0.016f; // ~60 FPS

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    // Create instanced mesh components for each robot type
    ScoutMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ScoutMeshes"));
    ScoutMeshComponent->SetupAttachment(RootComponent);
    ScoutMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ScoutMeshComponent->SetCastShadow(false);

    TransportMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TransportMeshes"));
    TransportMeshComponent->SetupAttachment(RootComponent);
    TransportMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    TransportMeshComponent->SetCastShadow(false);

    CombatMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CombatMeshes"));
    CombatMeshComponent->SetupAttachment(RootComponent);
    CombatMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CombatMeshComponent->SetCastShadow(false);

    RelayMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RelayMeshes"));
    RelayMeshComponent->SetupAttachment(RootComponent);
    RelayMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RelayMeshComponent->SetCastShadow(false);

    CoordinatorMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CoordinatorMeshes"));
    CoordinatorMeshComponent->SetupAttachment(RootComponent);
    CoordinatorMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CoordinatorMeshComponent->SetCastShadow(false);
}

void AAutonomySimRPCServer::BeginPlay() {
    Super::BeginPlay();

    UE_LOG(LogAutonomySimRPC, Log, TEXT("AutonomySim RPC Server initializing..."));

    // Set up meshes
    if (DroneMesh) {
        ScoutMeshComponent->SetStaticMesh(DroneMesh);
        CombatMeshComponent->SetStaticMesh(DroneMesh);
        RelayMeshComponent->SetStaticMesh(DroneMesh);
        CoordinatorMeshComponent->SetStaticMesh(DroneMesh);
    }

    if (GroundVehicleMesh) {
        TransportMeshComponent->SetStaticMesh(GroundVehicleMesh);
    }

    // Auto-start server if configured
    if (bAutoStartServer) {
        if (StartServer()) {
            UE_LOG(LogAutonomySimRPC, Log, TEXT("RPC Server started on port %d"), ListenPort);
        } else {
            UE_LOG(LogAutonomySimRPC, Error, TEXT("Failed to start RPC Server on port %d"), ListenPort);
        }
    }
}

void AAutonomySimRPCServer::EndPlay(const EEndPlayReason::Type EndPlayReason) {
    StopServer();
    Super::EndPlay(EndPlayReason);
}

void AAutonomySimRPCServer::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    // Process any queued RPC commands on the game thread
    ProcessRPCCommands();

    // Draw debug visualization
    if (bShowCommLinks && DebugLines.Num() > 0) {
        for (const FDebugLineData &Line : DebugLines) {
            DrawDebugLine(GetWorld(), Line.Start, Line.End, Line.Color.ToFColor(true), false, 0.0f, 0, Line.Thickness);
        }
    }
}

bool AAutonomySimRPCServer::StartServer() {
    if (bIsRunning) {
        UE_LOG(LogAutonomySimRPC, Warning, TEXT("Server already running"));
        return true;
    }

    // Create listen socket
    ISocketSubsystem *SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    if (!SocketSubsystem) {
        UE_LOG(LogAutonomySimRPC, Error, TEXT("Failed to get socket subsystem"));
        return false;
    }

    ListenSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("AutonomySim RPC"), false);
    if (!ListenSocket) {
        UE_LOG(LogAutonomySimRPC, Error, TEXT("Failed to create listen socket"));
        return false;
    }

    // Bind to port
    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    Addr->SetAnyAddress();
    Addr->SetPort(ListenPort);

    if (!ListenSocket->Bind(*Addr)) {
        UE_LOG(LogAutonomySimRPC, Error, TEXT("Failed to bind to port %d"), ListenPort);
        SocketSubsystem->DestroySocket(ListenSocket);
        ListenSocket = nullptr;
        return false;
    }

    // Start listening
    if (!ListenSocket->Listen(1)) {
        UE_LOG(LogAutonomySimRPC, Error, TEXT("Failed to listen on socket"));
        SocketSubsystem->DestroySocket(ListenSocket);
        ListenSocket = nullptr;
        return false;
    }

    // Create and start network thread
    NetworkRunnable = MakeShared<FAutonomySimRPCRunnable>(this);
    NetworkThread = FRunnableThread::Create(NetworkRunnable.Get(), TEXT("AutonomySim RPC Thread"));

    bIsRunning = true;
    UE_LOG(LogAutonomySimRPC, Log, TEXT("AutonomySim RPC Server listening on port %d"), ListenPort);
    return true;
}

void AAutonomySimRPCServer::StopServer() {
    if (!bIsRunning) {
        return;
    }

    UE_LOG(LogAutonomySimRPC, Log, TEXT("Stopping RPC Server..."));

    bIsRunning = false;

    // Stop network thread
    if (NetworkRunnable.IsValid()) {
        NetworkRunnable->Stop();
    }

    if (NetworkThread) {
        NetworkThread->Kill(true);
        delete NetworkThread;
        NetworkThread = nullptr;
    }

    NetworkRunnable.Reset();

    // Close sockets
    ISocketSubsystem *SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    if (SocketSubsystem) {
        if (ClientSocket) {
            SocketSubsystem->DestroySocket(ClientSocket);
            ClientSocket = nullptr;
        }
        if (ListenSocket) {
            SocketSubsystem->DestroySocket(ListenSocket);
            ListenSocket = nullptr;
        }
    }

    UE_LOG(LogAutonomySimRPC, Log, TEXT("RPC Server stopped"));
}

void AAutonomySimRPCServer::ProcessRPCCommands() {
    FRPCCommand Command;
    while (CommandQueue.Dequeue(Command)) {
        // Route command to appropriate handler
        if (Command.Method == TEXT("spawn_robots")) {
            HandleSpawnRobots(Command.Params, Command.RequestId);
        } else if (Command.Method == TEXT("update_positions")) {
            HandleUpdatePositions(Command.Params, Command.RequestId);
        } else if (Command.Method == TEXT("update_telemetry")) {
            HandleUpdateTelemetry(Command.Params, Command.RequestId);
        } else if (Command.Method == TEXT("simulation_step")) {
            HandleSimulationStep(Command.Params, Command.RequestId);
        } else if (Command.Method == TEXT("clear_all_robots")) {
            HandleClearAllRobots(Command.RequestId);
        } else if (Command.Method == TEXT("draw_debug_lines")) {
            HandleDrawDebugLines(Command.Params, Command.RequestId);
        } else if (Command.Method == TEXT("draw_debug_spheres")) {
            HandleDrawDebugSpheres(Command.Params, Command.RequestId);
        } else if (Command.Method == TEXT("clear_debug")) {
            HandleClearDebug(Command.RequestId);
        } else if (Command.Method == TEXT("set_visualization_mode")) {
            HandleSetVisualizationMode(Command.Params, Command.RequestId);
        } else if (Command.Method == TEXT("pause")) {
            HandlePause(Command.RequestId);
        } else if (Command.Method == TEXT("resume")) {
            HandleResume(Command.RequestId);
        } else if (Command.Method == TEXT("reset")) {
            HandleReset(Command.RequestId);
        } else {
            UE_LOG(LogAutonomySimRPC, Warning, TEXT("Unknown RPC method: %s"), *Command.Method);
            SendResponse(Command.RequestId, false, TEXT("Unknown method"));
        }
    }
}

void AAutonomySimRPCServer::HandleSpawnRobots(const TSharedPtr<FJsonObject> &Params, int32 RequestId) {
    const TArray<TSharedPtr<FJsonValue>> *RobotsArray;
    if (!Params->TryGetArrayField(TEXT("robots"), RobotsArray)) {
        SendResponse(RequestId, false, TEXT("Missing 'robots' array"));
        return;
    }

    int32 SpawnCount = 0;
    for (const TSharedPtr<FJsonValue> &RobotValue : *RobotsArray) {
        const TSharedPtr<FJsonObject> *RobotObj;
        if (!RobotValue->TryGetObject(RobotObj)) {
            continue;
        }

        FRobotInstanceData Data;
        Data.RobotId = (*RobotObj)->GetIntegerField(TEXT("id"));
        Data.Position.X = (*RobotObj)->GetNumberField(TEXT("x"));
        Data.Position.Y = (*RobotObj)->GetNumberField(TEXT("y"));
        Data.Position.Z = (*RobotObj)->GetNumberField(TEXT("z"));
        Data.Rotation.Yaw = (*RobotObj)->GetNumberField(TEXT("yaw"));

        // Parse robot type if provided
        FString TypeStr;
        if ((*RobotObj)->TryGetStringField(TEXT("robot_type"), TypeStr)) {
            if (TypeStr == TEXT("Scout"))
                Data.RobotType = EAutonomySimRobotType::Scout;
            else if (TypeStr == TEXT("Transport"))
                Data.RobotType = EAutonomySimRobotType::Transport;
            else if (TypeStr == TEXT("Combat"))
                Data.RobotType = EAutonomySimRobotType::Combat;
            else if (TypeStr == TEXT("Relay"))
                Data.RobotType = EAutonomySimRobotType::Relay;
            else if (TypeStr == TEXT("Coordinator"))
                Data.RobotType = EAutonomySimRobotType::Coordinator;
        }

        Robots.Add(Data.RobotId, Data);
        SpawnCount++;

        // Notify Blueprint
        OnRobotSpawned(Data.RobotId, Data.RobotType, Data.Position);
    }

    // Update instanced meshes
    UpdateInstancedMeshes();

    UE_LOG(LogAutonomySimRPC, Log, TEXT("Spawned %d robots (total: %d)"), SpawnCount, Robots.Num());
    SendResponse(RequestId, true);
}

void AAutonomySimRPCServer::HandleUpdatePositions(const TSharedPtr<FJsonObject> &Params, int32 RequestId) {
    const TArray<TSharedPtr<FJsonValue>> *PositionsArray;
    if (!Params->TryGetArrayField(TEXT("positions"), PositionsArray)) {
        SendResponse(RequestId, false, TEXT("Missing 'positions' array"));
        return;
    }

    int32 UpdateCount = 0;
    for (const TSharedPtr<FJsonValue> &PosValue : *PositionsArray) {
        const TSharedPtr<FJsonObject> *PosObj;
        if (!PosValue->TryGetObject(PosObj)) {
            continue;
        }

        int32 Id = (*PosObj)->GetIntegerField(TEXT("id"));
        if (FRobotInstanceData *Data = Robots.Find(Id)) {
            Data->Position.X = (*PosObj)->GetNumberField(TEXT("x"));
            Data->Position.Y = (*PosObj)->GetNumberField(TEXT("y"));
            Data->Position.Z = (*PosObj)->GetNumberField(TEXT("z"));
            Data->Rotation.Yaw = (*PosObj)->GetNumberField(TEXT("yaw"));
            Data->Rotation.Pitch = (*PosObj)->GetNumberField(TEXT("pitch"));
            Data->Rotation.Roll = (*PosObj)->GetNumberField(TEXT("roll"));
            UpdateCount++;
        }
    }

    // Update instanced meshes
    UpdateInstancedMeshes();

    OnPositionsUpdated(UpdateCount);
    SendResponse(RequestId, true);
}

void AAutonomySimRPCServer::HandleUpdateTelemetry(const TSharedPtr<FJsonObject> &Params, int32 RequestId) {
    const TArray<TSharedPtr<FJsonValue>> *TelemetryArray;
    if (!Params->TryGetArrayField(TEXT("telemetry"), TelemetryArray)) {
        SendResponse(RequestId, false, TEXT("Missing 'telemetry' array"));
        return;
    }

    for (const TSharedPtr<FJsonValue> &TelValue : *TelemetryArray) {
        const TSharedPtr<FJsonObject> *TelObj;
        if (!TelValue->TryGetObject(TelObj)) {
            continue;
        }

        int32 Id = (*TelObj)->GetIntegerField(TEXT("id"));
        if (FRobotInstanceData *Data = Robots.Find(Id)) {
            Data->BatteryPercent = (*TelObj)->GetNumberField(TEXT("battery_percent"));
            Data->HealthPercent = (*TelObj)->GetNumberField(TEXT("health_percent"));
            Data->SignalStrengthDbm = (*TelObj)->GetNumberField(TEXT("signal_strength_dbm"));
            Data->bIsJammed = (*TelObj)->GetBoolField(TEXT("is_jammed"));
            Data->bIsActive = (*TelObj)->GetBoolField(TEXT("is_active"));
        }
    }

    SendResponse(RequestId, true);
}

void AAutonomySimRPCServer::HandleSimulationStep(const TSharedPtr<FJsonObject> &Params, int32 RequestId) {
    // Simulation step - mainly for synchronization
    // Position updates are handled separately
    SendResponse(RequestId, true);
}

void AAutonomySimRPCServer::HandleClearAllRobots(int32 RequestId) {
    Robots.Empty();

    // Clear all instanced meshes
    ScoutMeshComponent->ClearInstances();
    TransportMeshComponent->ClearInstances();
    CombatMeshComponent->ClearInstances();
    RelayMeshComponent->ClearInstances();
    CoordinatorMeshComponent->ClearInstances();

    UE_LOG(LogAutonomySimRPC, Log, TEXT("Cleared all robots"));
    SendResponse(RequestId, true);
}

void AAutonomySimRPCServer::HandleDrawDebugLines(const TSharedPtr<FJsonObject> &Params, int32 RequestId) {
    DebugLines.Empty();

    const TArray<TSharedPtr<FJsonValue>> *LinesArray;
    if (!Params->TryGetArrayField(TEXT("lines"), LinesArray)) {
        SendResponse(RequestId, false, TEXT("Missing 'lines' array"));
        return;
    }

    for (const TSharedPtr<FJsonValue> &LineValue : *LinesArray) {
        const TSharedPtr<FJsonObject> *LineObj;
        if (!LineValue->TryGetObject(LineObj)) {
            continue;
        }

        FDebugLineData Line;
        const TArray<TSharedPtr<FJsonValue>> *StartArray;
        const TArray<TSharedPtr<FJsonValue>> *EndArray;
        const TArray<TSharedPtr<FJsonValue>> *ColorArray;

        if ((*LineObj)->TryGetArrayField(TEXT("start"), StartArray) && StartArray->Num() >= 3) {
            Line.Start.X = (*StartArray)[0]->AsNumber();
            Line.Start.Y = (*StartArray)[1]->AsNumber();
            Line.Start.Z = (*StartArray)[2]->AsNumber();
        }

        if ((*LineObj)->TryGetArrayField(TEXT("end"), EndArray) && EndArray->Num() >= 3) {
            Line.End.X = (*EndArray)[0]->AsNumber();
            Line.End.Y = (*EndArray)[1]->AsNumber();
            Line.End.Z = (*EndArray)[2]->AsNumber();
        }

        if ((*LineObj)->TryGetArrayField(TEXT("color"), ColorArray) && ColorArray->Num() >= 4) {
            Line.Color.R = (*ColorArray)[0]->AsNumber() / 255.0f;
            Line.Color.G = (*ColorArray)[1]->AsNumber() / 255.0f;
            Line.Color.B = (*ColorArray)[2]->AsNumber() / 255.0f;
            Line.Color.A = (*ColorArray)[3]->AsNumber() / 255.0f;
        }

        Line.Thickness = (*LineObj)->GetNumberField(TEXT("thickness"));

        DebugLines.Add(Line);
    }

    SendResponse(RequestId, true);
}

void AAutonomySimRPCServer::HandleDrawDebugSpheres(const TSharedPtr<FJsonObject> &Params, int32 RequestId) {
    DebugSpheres.Empty();
    DebugSphereRadii.Empty();
    DebugSphereColors.Empty();

    const TArray<TSharedPtr<FJsonValue>> *SpheresArray;
    if (!Params->TryGetArrayField(TEXT("spheres"), SpheresArray)) {
        SendResponse(RequestId, false, TEXT("Missing 'spheres' array"));
        return;
    }

    for (const TSharedPtr<FJsonValue> &SphereValue : *SpheresArray) {
        const TSharedPtr<FJsonObject> *SphereObj;
        if (!SphereValue->TryGetObject(SphereObj)) {
            continue;
        }

        FVector Center = FVector::ZeroVector;
        const TArray<TSharedPtr<FJsonValue>> *CenterArray;
        if ((*SphereObj)->TryGetArrayField(TEXT("center"), CenterArray) && CenterArray->Num() >= 3) {
            Center.X = (*CenterArray)[0]->AsNumber();
            Center.Y = (*CenterArray)[1]->AsNumber();
            Center.Z = (*CenterArray)[2]->AsNumber();
        }

        float Radius = (*SphereObj)->GetNumberField(TEXT("radius"));

        FLinearColor Color = FLinearColor::White;
        const TArray<TSharedPtr<FJsonValue>> *ColorArray;
        if ((*SphereObj)->TryGetArrayField(TEXT("color"), ColorArray) && ColorArray->Num() >= 4) {
            Color.R = (*ColorArray)[0]->AsNumber() / 255.0f;
            Color.G = (*ColorArray)[1]->AsNumber() / 255.0f;
            Color.B = (*ColorArray)[2]->AsNumber() / 255.0f;
            Color.A = (*ColorArray)[3]->AsNumber() / 255.0f;
        }

        DebugSpheres.Add(Center);
        DebugSphereRadii.Add(Radius);
        DebugSphereColors.Add(Color);
    }

    SendResponse(RequestId, true);
}

void AAutonomySimRPCServer::HandleClearDebug(int32 RequestId) {
    DebugLines.Empty();
    DebugSpheres.Empty();
    DebugSphereRadii.Empty();
    DebugSphereColors.Empty();
    SendResponse(RequestId, true);
}

void AAutonomySimRPCServer::HandleSetVisualizationMode(const TSharedPtr<FJsonObject> &Params, int32 RequestId) {
    bShowCommLinks = Params->GetBoolField(TEXT("show_comm_links"));
    bShowRFRange = Params->GetBoolField(TEXT("show_rf_range"));
    bShowPaths = Params->GetBoolField(TEXT("show_paths"));
    bShowLabels = Params->GetBoolField(TEXT("show_labels"));

    UE_LOG(LogAutonomySimRPC, Log, TEXT("Visualization mode: CommLinks=%d, RFRange=%d, Paths=%d, Labels=%d"),
           bShowCommLinks, bShowRFRange, bShowPaths, bShowLabels);

    SendResponse(RequestId, true);
}

void AAutonomySimRPCServer::HandlePause(int32 RequestId) {
    bIsPaused = true;
    OnSimulationStateChanged(true);
    SendResponse(RequestId, true);
}

void AAutonomySimRPCServer::HandleResume(int32 RequestId) {
    bIsPaused = false;
    OnSimulationStateChanged(false);
    SendResponse(RequestId, true);
}

void AAutonomySimRPCServer::HandleReset(int32 RequestId) {
    Robots.Empty();
    DebugLines.Empty();
    DebugSpheres.Empty();
    DebugSphereRadii.Empty();
    DebugSphereColors.Empty();

    ScoutMeshComponent->ClearInstances();
    TransportMeshComponent->ClearInstances();
    CombatMeshComponent->ClearInstances();
    RelayMeshComponent->ClearInstances();
    CoordinatorMeshComponent->ClearInstances();

    bIsPaused = false;

    UE_LOG(LogAutonomySimRPC, Log, TEXT("Simulation reset"));
    SendResponse(RequestId, true);
}

void AAutonomySimRPCServer::SendResponse(int32 RequestId, bool bSuccess, const FString &ErrorMessage) {
    FString Response;
    if (bSuccess) {
        Response = FString::Printf(TEXT("{\"jsonrpc\":\"2.0\",\"id\":%d,\"result\":null}\n"), RequestId);
    } else {
        Response = FString::Printf(TEXT("{\"jsonrpc\":\"2.0\",\"id\":%d,\"error\":{\"code\":-1,\"message\":\"%s\"}}\n"),
                                   RequestId, *ErrorMessage);
    }

    ResponseQueue.Enqueue(Response);
}

void AAutonomySimRPCServer::UpdateInstancedMeshes() {
    // Clear all instances
    ScoutMeshComponent->ClearInstances();
    TransportMeshComponent->ClearInstances();
    CombatMeshComponent->ClearInstances();
    RelayMeshComponent->ClearInstances();
    CoordinatorMeshComponent->ClearInstances();

    // Add instances for each robot
    for (const auto &Pair : Robots) {
        const FRobotInstanceData &Data = Pair.Value;
        FTransform Transform(Data.Rotation, Data.Position, FVector::OneVector);

        UInstancedStaticMeshComponent *MeshComp = GetMeshComponentForType(Data.RobotType);
        if (MeshComp) {
            MeshComp->AddInstance(Transform);
        }
    }
}

bool AAutonomySimRPCServer::GetRobotData(int32 RobotId, FRobotInstanceData &OutData) const {
    if (const FRobotInstanceData *Data = Robots.Find(RobotId)) {
        OutData = *Data;
        return true;
    }
    return false;
}

FLinearColor AAutonomySimRPCServer::GetRobotTypeColor(EAutonomySimRobotType Type) const {
    switch (Type) {
    case EAutonomySimRobotType::Scout:
        return ScoutColor;
    case EAutonomySimRobotType::Transport:
        return TransportColor;
    case EAutonomySimRobotType::Combat:
        return CombatColor;
    case EAutonomySimRobotType::Relay:
        return RelayColor;
    case EAutonomySimRobotType::Coordinator:
        return CoordinatorColor;
    default:
        return FLinearColor::White;
    }
}

UInstancedStaticMeshComponent *AAutonomySimRPCServer::GetMeshComponentForType(EAutonomySimRobotType Type) const {
    switch (Type) {
    case EAutonomySimRobotType::Scout:
        return ScoutMeshComponent;
    case EAutonomySimRobotType::Transport:
        return TransportMeshComponent;
    case EAutonomySimRobotType::Combat:
        return CombatMeshComponent;
    case EAutonomySimRobotType::Relay:
        return RelayMeshComponent;
    case EAutonomySimRobotType::Coordinator:
        return CoordinatorMeshComponent;
    default:
        return nullptr;
    }
}

//=============================================================================
// FAutonomySimRPCRunnable Implementation
//=============================================================================

FAutonomySimRPCRunnable::FAutonomySimRPCRunnable(AAutonomySimRPCServer *InOwner) : Owner(InOwner), bShouldStop(false) {}

FAutonomySimRPCRunnable::~FAutonomySimRPCRunnable() {}

bool FAutonomySimRPCRunnable::Init() { return true; }

uint32 FAutonomySimRPCRunnable::Run() {
    UE_LOG(LogAutonomySimRPC, Log, TEXT("Network thread started"));

    while (!bShouldStop) {
        // Check for pending client connection
        if (Owner->ListenSocket && !Owner->ClientSocket) {
            bool bHasPending = false;
            if (Owner->ListenSocket->WaitForPendingConnection(bHasPending, FTimespan::FromMilliseconds(100))) {
                if (bHasPending) {
                    Owner->ClientSocket = Owner->ListenSocket->Accept(TEXT("AutonomySim Client"));
                    if (Owner->ClientSocket) {
                        UE_LOG(LogAutonomySimRPC, Log, TEXT("Client connected"));

                        // Notify on game thread
                        AsyncTask(ENamedThreads::GameThread, [this]() {
                            if (Owner && Owner->IsValidLowLevel()) {
                                Owner->OnClientConnected();
                            }
                        });
                    }
                }
            }
        }

        // Handle connected client
        if (Owner->ClientSocket) {
            uint32 PendingDataSize = 0;
            if (Owner->ClientSocket->HasPendingData(PendingDataSize) && PendingDataSize > 0) {
                TArray<uint8> Buffer;
                Buffer.SetNumUninitialized(PendingDataSize + 1);

                int32 BytesRead = 0;
                if (Owner->ClientSocket->Recv(Buffer.GetData(), PendingDataSize, BytesRead)) {
                    Buffer[BytesRead] = 0; // Null terminate
                    FString Message = UTF8_TO_TCHAR(Buffer.GetData());

                    // Process each line (messages are newline-delimited)
                    TArray<FString> Lines;
                    Message.ParseIntoArray(Lines, TEXT("\n"), true);

                    for (const FString &Line : Lines) {
                        if (!Line.IsEmpty()) {
                            ProcessMessage(Line);
                        }
                    }
                }
            }

            // Send any pending responses
            FString Response;
            while (Owner->ResponseQueue.Dequeue(Response)) {
                int32 BytesSent = 0;
                FTCHARToUTF8 Converter(*Response);
                Owner->ClientSocket->Send((const uint8 *)Converter.Get(), Converter.Length(), BytesSent);
            }

            // Check for disconnection
            if (Owner->ClientSocket->GetConnectionState() != SCS_Connected) {
                UE_LOG(LogAutonomySimRPC, Log, TEXT("Client disconnected"));

                ISocketSubsystem *SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
                if (SocketSubsystem) {
                    SocketSubsystem->DestroySocket(Owner->ClientSocket);
                }
                Owner->ClientSocket = nullptr;

                // Notify on game thread
                AsyncTask(ENamedThreads::GameThread, [this]() {
                    if (Owner && Owner->IsValidLowLevel()) {
                        Owner->OnClientDisconnected();
                    }
                });
            }
        }

        // Small sleep to prevent busy waiting
        FPlatformProcess::Sleep(0.001f);
    }

    UE_LOG(LogAutonomySimRPC, Log, TEXT("Network thread stopped"));
    return 0;
}

void FAutonomySimRPCRunnable::Stop() { bShouldStop = true; }

void FAutonomySimRPCRunnable::Exit() {}

void FAutonomySimRPCRunnable::ProcessMessage(const FString &Message) {
    // Parse JSON-RPC message
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid()) {
        UE_LOG(LogAutonomySimRPC, Warning, TEXT("Failed to parse JSON: %s"), *Message);
        return;
    }

    // Extract JSON-RPC fields
    FString Method = JsonObject->GetStringField(TEXT("method"));
    int32 RequestId = JsonObject->GetIntegerField(TEXT("id"));
    const TSharedPtr<FJsonObject> *Params = nullptr;
    JsonObject->TryGetObjectField(TEXT("params"), Params);

    // Queue command for game thread processing
    FRPCCommand Command;
    Command.Method = Method;
    Command.RequestId = RequestId;
    Command.Params = Params ? *Params : MakeShared<FJsonObject>();

    Owner->CommandQueue.Enqueue(Command);
}

void FAutonomySimRPCRunnable::QueueResponse(const FString &Response) { PendingResponses.Enqueue(Response); }
