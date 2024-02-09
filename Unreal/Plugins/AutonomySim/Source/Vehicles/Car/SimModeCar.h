#pragma once

#include "CoreMinimal.h"

#include "CarPawn.h"
#include "SimMode/SimModeBase.h"
#include "SimModeCar.generated.h"
#include "api/VehicleSimApiBase.hpp"
#include "common/Common.hpp"

UCLASS()
class AutonomySimApi ASimModeCar : public ASimModeBase {
    GENERATED_BODY()

  public:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    virtual void continueForTime(double seconds) override;
    virtual void continueForFrames(uint32_t frames) override;

  private:
    typedef nervosys::autonomylib::ClockFactory ClockFactory;
    typedef common_utils::Utils Utils;
    typedef nervosys::autonomylib::TTimePoint TTimePoint;
    typedef nervosys::autonomylib::TTimeDelta TTimeDelta;
    typedef ACarPawn TVehiclePawn;
    typedef nervosys::autonomylib::VehicleSimApiBase VehicleSimApiBase;
    typedef nervosys::autonomylib::VectorMath VectorMath;
    typedef nervosys::autonomylib::Vector3r Vector3r;

  private:
    void initializePauseState();

  protected:
    virtual void setupClockSpeed() override;
    virtual std::unique_ptr<nervosys::autonomylib::ApiServerBase> createApiServer() const override;
    virtual void getExistingVehiclePawns(TArray<AActor *> &pawns) const override;
    virtual bool isVehicleTypeSupported(const std::string &vehicle_type) const override;
    virtual std::string
    getVehiclePawnPathName(const AutonomySimSettings::VehicleSetting &vehicle_setting) const override;
    virtual PawnEvents *getVehiclePawnEvents(APawn *pawn) const override;
    virtual const common_utils::UniqueValueMap<std::string, APIPCamera *>
    getVehiclePawnCameras(APawn *pawn) const override;
    virtual void initializeVehiclePawn(APawn *pawn) override;
    virtual std::unique_ptr<PawnSimApi>
    createVehicleSimApi(const PawnSimApi::Params &pawn_sim_api_params) const override;
    virtual nervosys::autonomylib::VehicleApiBase *getVehicleApi(const PawnSimApi::Params &pawn_sim_api_params,
                                                                 const PawnSimApi *sim_api) const override;

  private:
    std::atomic<float> current_clockspeed_;
    std::atomic<TTimeDelta> pause_period_;
    std::atomic<TTimePoint> pause_period_start_;
    uint32_t targetFrameNumber_;
    std::atomic_bool frame_countdown_enabled_;
    ;
};
