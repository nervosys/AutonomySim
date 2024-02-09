#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/Material.h"
#include "TextureShuffleActor.generated.h"
#include "common/AutonomySimSettings.hpp"
#include "common/utils/Utils.hpp"

UCLASS()
class AutonomySimApi ATextureShuffleActor : public AStaticMeshActor {
    GENERATED_BODY()

  protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextureShuffle)
    UMaterialInterface *DynamicMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextureShuffle)
    TArray<UTexture2D *> SwappableTextures;

  public:
    UFUNCTION(BlueprintNativeEvent)
    void SwapTexture(int tex_id = 0, int component_id = 0, int material_id = 0);

  private:
    bool MaterialCacheInitialized = false;
    int NumComponents = -1;

    UPROPERTY()
    TArray<UMaterialInstanceDynamic *> DynamicMaterialInstances;
};