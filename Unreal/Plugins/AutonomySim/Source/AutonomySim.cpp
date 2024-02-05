// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "AutonomySim.h"
#include "Misc/Paths.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FAutonomySim : public IModuleInterface {
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FAutonomySim, AutonomySim)

void FAutonomySim::StartupModule() {
    // plugin startup
    UE_LOG(LogTemp, Log, TEXT("StartupModule: AutonomySim plugin"));
}

void FAutonomySim::ShutdownModule() {
    // plugin shutdown
}