#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PIPCamera.h"
#include "SimHUD.generated.h"
#include "SimHUDWidget.h"
#include "SimMode/SimModeBase.h"
#include "api/ApiServerBase.hpp"
#include <memory>

UENUM(BlueprintType)
enum class ESimulatorMode : uint8 { SIM_MODE_HIL UMETA(DisplayName = "Hardware-in-loop") };

UCLASS()
class AutonomySimApi ASimHUD : public AHUD {
    GENERATED_BODY()

  public:
    typedef nervosys::autonomylib::ImageCaptureBase::ImageType ImageType;
    typedef nervosys::autonomylib::AutonomySimSettings AutonomySimSettings;

  public:
    void inputEventToggleRecording();
    void inputEventToggleReport();
    void inputEventToggleHelp();
    void inputEventToggleTrace();
    void inputEventToggleSubwindow0();
    void inputEventToggleSubwindow1();
    void inputEventToggleSubwindow2();
    void inputEventToggleAll();

    ASimHUD();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

  protected:
    virtual void setupInputBindings();
    void toggleRecordHandler();
    void updateWidgetSubwindowVisibility();
    bool isWidgetSubwindowVisible(int window_index);
    void toggleSubwindowVisibility(int window_index);

  private:
    void initializeSubWindows();
    void createSimMode();
    void initializeSettings();
    void setUnrealEngineSettings();
    void loadLevel();
    void createMainWidget();
    const std::vector<AutonomySimSettings::SubwindowSetting> &getSubWindowSettings() const;
    std::vector<AutonomySimSettings::SubwindowSetting> &getSubWindowSettings();

    bool getSettingsText(std::string &settingsText);
    bool getSettingsTextFromCommandLine(std::string &settingsText);
    bool readSettingsTextFromFile(const FString &fileName, std::string &settingsText);
    std::string getSimModeFromUser();

    static FString getLaunchPath(const std::string &filename);

  private:
    typedef common_utils::Utils Utils;
    UClass *widget_class_;

    UPROPERTY()
    USimHUDWidget *widget_;
    UPROPERTY()
    ASimModeBase *simmode_;

    APIPCamera *subwindow_cameras_[AutonomySimSettings::kSubwindowCount];
    bool map_changed_;
};
