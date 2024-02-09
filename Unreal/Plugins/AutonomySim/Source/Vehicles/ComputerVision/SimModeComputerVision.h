#pragma once

#include "CoreMinimal.h"

#include "ComputerVisionPawn.h"
#include "SimMode/SimModeBase.h"
#include "api/VehicleSimApiBase.hpp"
#include "common/Common.hpp"

#include "SimModeComputerVision.generated.h"

UCLASS()
class AutonomySimApi ASimModeComputerVision : public ASimModeBase {
    GENERATED_BODY()

  private:
    typedef AComputerVisionPawn TVehiclePawn;

  protected:
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
};
