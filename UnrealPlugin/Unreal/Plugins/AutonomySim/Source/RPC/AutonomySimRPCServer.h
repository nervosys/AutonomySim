// AutonomySimRPCServer.h
// JSON-RPC server for AutonomySim Rust backend communication
//
// This actor receives commands from the Rust simulation backend
// and visualizes the robotic swarm in Unreal Engine 5.
//
// Protocol: JSON-RPC 2.0 over TCP on port 41451

#pragma once

#include "AutonomySimRPCServer.generated.h"
#include "Common/TcpListener.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Containers/Queue.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Networking.h"
#include "SocketSubsystem.h"
#include "Sockets.h"

// Forward declarations
class FAutonomySimRPCRunnable;

/**
 * Robot type enumeration matching Rust backend
 */
UENUM(BlueprintType)
enum class EAutonomySimRobotType : uint8 {
    Scout UMETA(DisplayName = "Scout"),            // Red
    Transport UMETA(DisplayName = "Transport"),    // Blue
    Combat UMETA(DisplayName = "Combat"),          // Green
    Relay UMETA(DisplayName = "Relay"),            // Yellow
    Coordinator UMETA(DisplayName = "Coordinator") // Purple
};

/**
 * Robot instance data for visualization
 */
USTRUCT(BlueprintType)
struct FRobotInstanceData {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    int32 RobotId = 0;

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    EAutonomySimRobotType RobotType = EAutonomySimRobotType::Scout;

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    float BatteryPercent = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    float HealthPercent = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    float SignalStrengthDbm = -50.0f;

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    bool bIsJammed = false;

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    bool bIsActive = true;
};

/**
 * Debug line for visualization
 */
USTRUCT(BlueprintType)
struct FDebugLineData {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    FVector Start = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    FVector End = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    FLinearColor Color = FLinearColor::White;

    UPROPERTY(BlueprintReadWrite, Category = "AutonomySim")
    float Thickness = 1.0f;
};

/**
 * RPC command received from Rust backend
 */
USTRUCT()
struct FRPCCommand {
    GENERATED_BODY()

    FString Method;
    TSharedPtr<FJsonObject> Params;
    int32 RequestId = 0;
};

/**
 * AutonomySim RPC Server Actor
 *
 * Place this actor in your level to enable communication between
 * the Rust AutonomySim backend and Unreal Engine visualization.
 *
 * The server listens on port 41451 for JSON-RPC commands.
 */
UCLASS(BlueprintType, Blueprintable)
class AUTONOMYSIM_API AAutonomySimRPCServer : public AActor {
    GENERATED_BODY()

  public:
    AAutonomySimRPCServer();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    // === Configuration ===

    /** TCP port to listen on (default: 41451) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Network")
    int32 ListenPort = 41451;

    /** Whether to start the server automatically on BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Network")
    bool bAutoStartServer = true;

    /** Maximum robots to visualize (for performance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Performance")
    int32 MaxRobots = 2000;

    // === Visualization Settings ===

    /** Show communication links between robots */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Visualization")
    bool bShowCommLinks = true;

    /** Show RF range spheres */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Visualization")
    bool bShowRFRange = false;

    /** Show robot paths */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Visualization")
    bool bShowPaths = false;

    /** Show robot labels */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Visualization")
    bool bShowLabels = false;

    // === Robot Type Colors ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Colors")
    FLinearColor ScoutColor = FLinearColor(1.0f, 0.25f, 0.25f, 1.0f); // Red

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Colors")
    FLinearColor TransportColor = FLinearColor(0.25f, 0.5f, 1.0f, 1.0f); // Blue

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Colors")
    FLinearColor CombatColor = FLinearColor(0.25f, 1.0f, 0.25f, 1.0f); // Green

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Colors")
    FLinearColor RelayColor = FLinearColor(1.0f, 1.0f, 0.25f, 1.0f); // Yellow

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Colors")
    FLinearColor CoordinatorColor = FLinearColor(0.75f, 0.25f, 1.0f, 1.0f); // Purple

    // === Robot Meshes ===

    /** Mesh to use for drone robots (Scout, Combat, Relay, Coordinator) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Meshes")
    UStaticMesh *DroneMesh;

    /** Mesh to use for ground vehicles (Transport) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutonomySim|Meshes")
    UStaticMesh *GroundVehicleMesh;

    // === Blueprint Events ===

    /** Called when a robot is spawned */
    UFUNCTION(BlueprintImplementableEvent, Category = "AutonomySim|Events")
    void OnRobotSpawned(int32 RobotId, EAutonomySimRobotType RobotType, FVector Position);

    /** Called when robot positions are updated */
    UFUNCTION(BlueprintImplementableEvent, Category = "AutonomySim|Events")
    void OnPositionsUpdated(int32 Count);

    /** Called when simulation is paused/resumed */
    UFUNCTION(BlueprintImplementableEvent, Category = "AutonomySim|Events")
    void OnSimulationStateChanged(bool bIsPaused);

    /** Called when a client connects */
    UFUNCTION(BlueprintImplementableEvent, Category = "AutonomySim|Events")
    void OnClientConnected();

    /** Called when a client disconnects */
    UFUNCTION(BlueprintImplementableEvent, Category = "AutonomySim|Events")
    void OnClientDisconnected();

    // === Blueprint Functions ===

    /** Start the RPC server */
    UFUNCTION(BlueprintCallable, Category = "AutonomySim")
    bool StartServer();

    /** Stop the RPC server */
    UFUNCTION(BlueprintCallable, Category = "AutonomySim")
    void StopServer();

    /** Check if server is running */
    UFUNCTION(BlueprintPure, Category = "AutonomySim")
    bool IsServerRunning() const { return bIsRunning; }

    /** Get the number of robots currently visualized */
    UFUNCTION(BlueprintPure, Category = "AutonomySim")
    int32 GetRobotCount() const { return Robots.Num(); }

    /** Get robot data by ID */
    UFUNCTION(BlueprintPure, Category = "AutonomySim")
    bool GetRobotData(int32 RobotId, FRobotInstanceData &OutData) const;

    /** Get color for robot type */
    UFUNCTION(BlueprintPure, Category = "AutonomySim")
    FLinearColor GetRobotTypeColor(EAutonomySimRobotType Type) const;

  protected:
    // Process queued RPC commands on game thread
    void ProcessRPCCommands();

    // RPC command handlers
    void HandleSpawnRobots(const TSharedPtr<FJsonObject> &Params, int32 RequestId);
    void HandleUpdatePositions(const TSharedPtr<FJsonObject> &Params, int32 RequestId);
    void HandleUpdateTelemetry(const TSharedPtr<FJsonObject> &Params, int32 RequestId);
    void HandleSimulationStep(const TSharedPtr<FJsonObject> &Params, int32 RequestId);
    void HandleClearAllRobots(int32 RequestId);
    void HandleDrawDebugLines(const TSharedPtr<FJsonObject> &Params, int32 RequestId);
    void HandleDrawDebugSpheres(const TSharedPtr<FJsonObject> &Params, int32 RequestId);
    void HandleClearDebug(int32 RequestId);
    void HandleSetVisualizationMode(const TSharedPtr<FJsonObject> &Params, int32 RequestId);
    void HandlePause(int32 RequestId);
    void HandleResume(int32 RequestId);
    void HandleReset(int32 RequestId);

    // Send RPC response
    void SendResponse(int32 RequestId, bool bSuccess, const FString &ErrorMessage = TEXT(""));

    // Update instanced mesh rendering
    void UpdateInstancedMeshes();

  private:
    // Server state
    bool bIsRunning = false;
    bool bIsPaused = false;

    // Socket handling
    FSocket *ListenSocket = nullptr;
    FSocket *ClientSocket = nullptr;
    FRunnableThread *NetworkThread = nullptr;
    TSharedPtr<FAutonomySimRPCRunnable> NetworkRunnable;

    // Command queue (thread-safe)
    TQueue<FRPCCommand, EQueueMode::Mpsc> CommandQueue;
    TQueue<FString, EQueueMode::Mpsc> ResponseQueue;

    // Robot data
    TMap<int32, FRobotInstanceData> Robots;

    // Debug visualization
    TArray<FDebugLineData> DebugLines;
    TArray<FVector> DebugSpheres;
    TArray<float> DebugSphereRadii;
    TArray<FLinearColor> DebugSphereColors;

    // Instanced mesh components for each robot type
    UPROPERTY()
    UInstancedStaticMeshComponent *ScoutMeshComponent;

    UPROPERTY()
    UInstancedStaticMeshComponent *TransportMeshComponent;

    UPROPERTY()
    UInstancedStaticMeshComponent *CombatMeshComponent;

    UPROPERTY()
    UInstancedStaticMeshComponent *RelayMeshComponent;

    UPROPERTY()
    UInstancedStaticMeshComponent *CoordinatorMeshComponent;

    // Helper to get mesh component for robot type
    UInstancedStaticMeshComponent *GetMeshComponentForType(EAutonomySimRobotType Type) const;
};

/**
 * Network runnable for background socket handling
 */
class FAutonomySimRPCRunnable : public FRunnable {
  public:
    FAutonomySimRPCRunnable(AAutonomySimRPCServer *InOwner);
    virtual ~FAutonomySimRPCRunnable();

    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;
    virtual void Exit() override;

    // Thread-safe response sending
    void QueueResponse(const FString &Response);

  private:
    AAutonomySimRPCServer *Owner;
    FThreadSafeBool bShouldStop;
    TQueue<FString, EQueueMode::Mpsc> PendingResponses;

    // Parse JSON-RPC message and queue command
    void ProcessMessage(const FString &Message);
};
