#pragma once

#include "Camera/CameraActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "CoreMinimal.h"
#include "DetectionComponent.h"
#include "Materials/Material.h"
#include "NedTransform.h"
#include "Runtime/Core/Public/PixelFormat.h"
#include "common/AutonomySimSettings.hpp"
#include "common/ImageCaptureBase.hpp"
#include "common/common_utils/Utils.hpp"

// CinemAutonomySim
#include <CineCameraActor.h>
#include <CineCameraComponent.h>

#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "PIPCamera.generated.h"

UCLASS()
class AutonomySim_API APIPCamera : public ACineCameraActor // CinemAutonomySim
{
    GENERATED_BODY()

  public:
    typedef nervosys::autonomylib::ImageCaptureBase::ImageType ImageType;
    typedef nervosys::autonomylib::AutonomySimSettings AutonomySimSettings;
    typedef AutonomySimSettings::CameraSetting CameraSetting;

    APIPCamera(const FObjectInitializer &ObjectInitializer); // CinemAutonomySim

    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    void showToScreen();
    void disableAll();
    void disableAllPIP();
    void disableMain();
    void onViewModeChanged(bool nodisplay);

    // CinemAutonomySim methods
    std::vector<std::string> getPresetLensSettings() const;
    void setPresetLensSettings(std::string preset_string);
    std::vector<std::string> getPresetFilmbackSettings() const;
    void setPresetFilmbackSettings(std::string preset_string);
    std::string getLensSettings() const;
    std::string getFilmbackSettings() const;
    float setFilmbackSettings(float sensor_width, float sensor_height);
    float getFocalLength() const;
    void setFocalLength(float focal_length);
    void enableManualFocus(bool enable);
    float getFocusDistance() const;
    void setFocusDistance(float focus_distance);
    float getFocusAperture() const;
    void setFocusAperture(float focus_aperture);
    void enableFocusPlane(bool enable);
    std::string getCurrentFieldOfView() const;
    // end CinemAutonomySim methods

    void setCameraTypeEnabled(ImageType type, bool enabled);
    bool getCameraTypeEnabled(ImageType type) const;
    void setCaptureUpdate(USceneCaptureComponent2D *capture, bool nodisplay);
    void setCameraTypeUpdate(ImageType type, bool nodisplay);

    void setupCameraFromSettings(const CameraSetting &camera_setting, const NedTransform &ned_transform);
    void setCameraPose(const nervosys::autonomylib::Pose &relative_pose);
    void setCameraFoV(float fov_degrees);
    nervosys::autonomylib::CameraInfo getCameraInfo() const;
    std::vector<float> getDistortionParams() const;
    void setDistortionParam(const std::string &param_name, float value);

    nervosys::autonomylib::ProjectionMatrix getProjectionMatrix(const ImageType image_type) const;

    USceneCaptureComponent2D *getCaptureComponent(const ImageType type, bool if_active);
    UTextureRenderTarget2D *getRenderTarget(const ImageType type, bool if_active);
    UDetectionComponent *getDetectionComponent(const ImageType type, bool if_active) const;

    nervosys::autonomylib::Pose getPose() const;

  private: // members
    UPROPERTY()
    UMaterialParameterCollection *distortion_param_collection_;
    UPROPERTY()
    UMaterialParameterCollectionInstance *distortion_param_instance_;

    UPROPERTY()
    TArray<USceneCaptureComponent2D *> captures_;
    UPROPERTY()
    TArray<UTextureRenderTarget2D *> render_targets_;
    UPROPERTY()
    TArray<UDetectionComponent *> detections_;

    // CinemAutonomySim
    UPROPERTY()
    UCineCameraComponent *camera_;
    // TMap<int, UMaterialInstanceDynamic*> noise_materials_;
    // below is needed because TMap doesn't work with UPROPERTY, but we do have -ve index
    UPROPERTY()
    TArray<UMaterialInstanceDynamic *> noise_materials_;
    UPROPERTY()
    TArray<UMaterialInstanceDynamic *> distortion_materials_;
    UPROPERTY()
    UMaterial *noise_material_static_;
    UPROPERTY()
    UMaterial *distortion_material_static_;

    std::vector<bool> camera_type_enabled_;
    FRotator gimbald_rotator_;
    float gimbal_stabilization_;
    const NedTransform *ned_transform_;
    TMap<int, EPixelFormat> image_type_to_pixel_format_map_;

    FObjectFilter object_filter_;

  private: // methods
    typedef common_utils::Utils Utils;
    typedef AutonomySimSettings::CaptureSetting CaptureSetting;
    typedef AutonomySimSettings::NoiseSetting NoiseSetting;

    static unsigned int imageTypeCount();
    void enableCaptureComponent(const ImageType type, bool is_enabled);
    static void updateCaptureComponentSetting(USceneCaptureComponent2D *capture, UTextureRenderTarget2D *render_target,
                                              bool auto_format, const EPixelFormat &pixel_format,
                                              const CaptureSetting &setting, const NedTransform &ned_transform,
                                              bool force_linear_gamma);
    void setNoiseMaterial(int image_type, UObject *outer, FPostProcessSettings &obj, const NoiseSetting &settings);
    void setDistortionMaterial(int image_type, UObject *outer, FPostProcessSettings &obj);
    static void updateCameraPostProcessingSetting(FPostProcessSettings &obj, const CaptureSetting &setting);
    // CinemAutonomySim
    static void updateCameraSetting(UCineCameraComponent *camera, const CaptureSetting &setting,
                                    const NedTransform &ned_transform);
    void copyCameraSettingsToAllSceneCapture(UCameraComponent *camera);
    void copyCameraSettingsToSceneCapture(UCameraComponent *src, USceneCaptureComponent2D *dst);
    // end CinemAutonomySim
};
