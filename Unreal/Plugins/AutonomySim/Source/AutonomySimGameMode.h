// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AutonomySimGameMode.generated.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameUserSettings.h"

/**
 *
 */
UCLASS()
class AutonomySimApi AAutonomySimGameMode : public AGameModeBase {
  public:
    GENERATED_BODY()

    virtual void StartPlay() override;

    AAutonomySimGameMode(const FObjectInitializer &ObjectInitializer);

    // private:
    // UGameUserSettings* GetGameUserSettings();
};
