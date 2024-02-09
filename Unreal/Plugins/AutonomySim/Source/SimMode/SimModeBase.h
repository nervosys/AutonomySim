#pragma once

#include "Components/SkyLightComponent.h"
#include "CoreMinimal.h"
#include "Engine/DirectionalLight.h"
#include "GameFramework/Actor.h"
#include "ParticleDefinitions.h"

#include "CameraDirector.h"
#include "LoadingScreenWidget.h"
#include "PawnSimApi.h"
#include "SimModeBase.generated.h"
#include "UnrealImageCapture.h"
#include "api/ApiProvider.hpp"
#include "api/ApiServerBase.hpp"
#include "common/AutonomySimSettings.hpp"
#include "common/ClockFactory.hpp"
#include "common/StateReporterWrapper.hpp"
#include <string>

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLevelLoaded);

UCLASS()
class AutonomySimApi ASimModeBase : public AActor {
  public:
    GENERATED_BODY()

    UPROPERTY(BlueprintAssignable, BlueprintCallable)
    FLevelLoaded OnLevelLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Refs")
    ACameraDirector *CameraDirector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debugging")
    bool EnableReport = false;

    UFUNCTION(BlueprintCallable, Category = "Recording")
    bool toggleRecording();

    UFUNCTION(BlueprintPure, Category = "AutonomySim | get stuff")
    static ASimModeBase *getSimMode();

    UFUNCTION(BlueprintCallable, Category = "AutonomySim | get stuff")
    void toggleLoadingScreen(bool is_visible);

    UFUNCTION(BlueprintCallable, Category = "AutonomySim | get stuff")
    virtual void reset();

    // Sets default values for this actor's properties
    ASimModeBase();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    // additional overridable methods
    virtual std::string getDebugReport();
    virtual ECameraDirectorMode getInitialViewMode() const;

    virtual bool isPaused() const;
    virtual void pause(bool is_paused);
    virtual void continueForTime(double seconds);
    virtual void continueForFrames(uint32_t frames);

    virtual void setWind(const nervosys::autonomylib::Vector3r &wind) const;
    virtual void setExtForce(const nervosys::autonomylib::Vector3r &ext_force) const;

    virtual void setTimeOfDay(bool is_enabled, const std::string &start_datetime, bool is_start_datetime_dst,
                              float celestial_clock_speed, float update_interval_secs, bool move_sun);

    virtual void startRecording();
    virtual void stopRecording();
    virtual bool isRecording() const;

    virtual void toggleTraceAll();

    void startApiServer();
    void stopApiServer();
    bool isApiServerStarted();

    bool createVehicleAtRuntime(const std::string &vehicle_name, const std::string &vehicle_type,
                                const nervosys::autonomylib::Pose &pose, const std::string &pawn_path = "");

    const NedTransform &getGlobalNedTransform();

    nervosys::autonomylib::ApiProvider *getApiProvider() const { return api_provider_.get(); }
    const PawnSimApi *getVehicleSimApi(const std::string &vehicle_name = "") const {
        return static_cast<PawnSimApi *>(api_provider_->getVehicleSimApi(vehicle_name));
    }
    PawnSimApi *getVehicleSimApi(const std::string &vehicle_name = "") {
        return static_cast<PawnSimApi *>(api_provider_->getVehicleSimApi(vehicle_name));
    }

    const APIPCamera *getExternalCamera(const std::string &camera_name) const {
        return external_cameras_.findOrDefault(camera_name, nullptr);
    }

    APIPCamera *getExternalCamera(const std::string &camera_name) {
        return const_cast<APIPCamera *>(static_cast<const ASimModeBase *>(this)->getExternalCamera(camera_name));
    }

    const APIPCamera *getCamera(const nervosys::autonomylib::CameraDetails &camera_details) const;

    APIPCamera *getCamera(const nervosys::autonomylib::CameraDetails &camera_details) {
        return const_cast<APIPCamera *>(static_cast<const ASimModeBase *>(this)->getCamera(camera_details));
    }

    const UnrealImageCapture *getExternalImageCapture() const { return external_image_capture_.get(); }

    const UnrealImageCapture *getImageCapture(const std::string &vehicle_name = "", bool external = false) const;

    TMap<FString, FAssetData> asset_map;
    TMap<FString, AActor *> scene_object_map;
    UMaterial *domain_rand_material_;

  protected: // must overrides
    typedef nervosys::autonomylib::AutonomySimSettings AutonomySimSettings;

    virtual std::unique_ptr<nervosys::autonomylib::ApiServerBase> createApiServer() const;
    virtual void getExistingVehiclePawns(TArray<AActor *> &pawns) const;
    virtual bool isVehicleTypeSupported(const std::string &vehicle_type) const;
    virtual std::string getVehiclePawnPathName(const AutonomySimSettings::VehicleSetting &vehicle_setting) const;
    virtual PawnEvents *getVehiclePawnEvents(APawn *pawn) const;
    virtual const common_utils::UniqueValueMap<std::string, APIPCamera *> getVehiclePawnCameras(APawn *pawn) const;
    virtual void initializeVehiclePawn(APawn *pawn);
    virtual std::unique_ptr<PawnSimApi> createVehicleSimApi(const PawnSimApi::Params &pawn_sim_api_params) const;
    virtual nervosys::autonomylib::VehicleApiBase *getVehicleApi(const PawnSimApi::Params &pawn_sim_api_params,
                                                                 const PawnSimApi *sim_api) const;
    virtual void registerPhysicsBody(nervosys::autonomylib::VehicleSimApiBase *physicsBody);

  protected: // optional overrides
    virtual APawn *createVehiclePawn(const AutonomySimSettings::VehicleSetting &vehicle_setting);
    virtual std::unique_ptr<PawnSimApi> createVehicleApi(APawn *vehicle_pawn);
    virtual void setupVehiclesAndCamera();
    virtual void setupInputBindings();
    // called when SimMode should handle clock speed setting
    virtual void setupClockSpeed();
    void initializeCameraDirector(const FTransform &camera_transform, float follow_distance);
    void checkVehicleReady(); // checks if vehicle is available to use
    virtual void updateDebugReport(nervosys::autonomylib::StateReporterWrapper &debug_reporter);
    virtual void initializeExternalCameras();

  protected: // Utility methods for derived classes
    virtual const AutonomySimSettings &getSettings() const;
    FRotator toFRotator(const AutonomySimSettings::Rotation &rotation, const FRotator &default_val);

  protected:
    int record_tick_count;
    UPROPERTY()
    UClass *pip_camera_class;
    UPROPERTY()
    UParticleSystem *collision_display_template;

  private:
    typedef common_utils::Utils Utils;
    typedef nervosys::autonomylib::ClockFactory ClockFactory;
    typedef nervosys::autonomylib::TTimePoint TTimePoint;
    typedef nervosys::autonomylib::TTimeDelta TTimeDelta;
    typedef nervosys::autonomylib::SensorBase::SensorType SensorType;
    typedef nervosys::autonomylib::Vector3r Vector3r;
    typedef nervosys::autonomylib::Pose Pose;
    typedef nervosys::autonomylib::VectorMath VectorMath;

  private:
    // assets loaded in constructor
    UPROPERTY()
    UClass *external_camera_class_;
    UPROPERTY()
    UClass *camera_director_class_;
    UPROPERTY()
    UClass *sky_sphere_class_;
    UPROPERTY()
    ULoadingScreenWidget *loading_screen_widget_;

    UPROPERTY()
    AActor *sky_sphere_;
    UPROPERTY()
    ADirectionalLight *sun_;
    FRotator default_sun_rotation_;
    TTimePoint tod_sim_clock_start_; // sim start in local time
    TTimePoint tod_last_update_;
    TTimePoint tod_start_time_; // tod, configurable
    bool tod_enabled_;
    float tod_celestial_clock_speed_;
    float tod_update_interval_secs_;
    bool tod_move_sun_;

    std::unique_ptr<NedTransform> global_ned_transform_;
    std::unique_ptr<nervosys::autonomylib::WorldSimApiBase> world_sim_api_;
    std::unique_ptr<nervosys::autonomylib::ApiProvider> api_provider_;
    std::unique_ptr<nervosys::autonomylib::ApiServerBase> api_server_;
    nervosys::autonomylib::StateReporterWrapper debug_reporter_;

    std::vector<std::unique_ptr<nervosys::autonomylib::VehicleSimApiBase>> vehicle_sim_apis_;
    common_utils::UniqueValueMap<std::string, APIPCamera *> external_cameras_;
    std::unique_ptr<UnrealImageCapture> external_image_capture_;

    UPROPERTY()
    TArray<AActor *> spawned_actors_; // keep refs alive from Unreal GC

    bool lidar_checks_done_ = false;
    bool lidar_draw_debug_points_ = false;
    static ASimModeBase *SIMMODE;

  private:
    void setStencilIDs();
    void initializeTimeOfDay();
    void advanceTimeOfDay();
    void setSunRotation(FRotator rotation);
    void setupPhysicsLoopPeriod();
    void showClockStats();
    void drawLidarDebugPoints();
    void drawDistanceSensorDebugPoints();
};
