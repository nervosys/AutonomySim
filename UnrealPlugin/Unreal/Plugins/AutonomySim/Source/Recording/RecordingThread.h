#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

#include "AutonomyBlueprintLib.h"
#include "Recording/RecordingFile.h"
#include "api/VehicleSimApiBase.hpp"
#include "common/AutonomySimSettings.hpp"
#include "common/ClockFactory.hpp"
#include "common/WorkerThread.hpp"
#include "physics/Kinematics.hpp"
#include <memory>

class FRecordingThread : public FRunnable {
  public:
    typedef nervosys::autonomylib::AutonomySimSettings::RecordingSetting RecordingSetting;
    typedef nervosys::autonomylib::VehicleSimApiBase VehicleSimApiBase;
    typedef nervosys::autonomylib::ImageCaptureBase ImageCaptureBase;

  public:
    FRecordingThread();
    virtual ~FRecordingThread();

    static void init();
    static void startRecording(const RecordingSetting &settings,
                               const common_utils::UniqueValueMap<std::string, VehicleSimApiBase *> &vehicle_sim_apis);
    static void stopRecording();
    static void killRecording();
    static bool isRecording();

  protected:
    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;
    virtual void Exit() override;

  private:
    FThreadSafeCounter stop_task_counter_;

    static std::unique_ptr<FRecordingThread> running_instance_;
    static std::unique_ptr<FRecordingThread> finishing_instance_;
    static nervosys::autonomylib::WorkerThreadSignal finishing_signal_;
    static bool first_;

    static std::unique_ptr<FRecordingThread> instance_;

    std::unique_ptr<FRunnableThread> thread_;

    RecordingSetting settings_;
    std::unique_ptr<RecordingFile> recording_file_;
    common_utils::UniqueValueMap<std::string, VehicleSimApiBase *> vehicle_sim_apis_;
    std::unordered_map<std::string, const ImageCaptureBase *> image_captures_;
    std::unordered_map<std::string, nervosys::autonomylib::Pose> last_poses_;

    nervosys::autonomylib::TTimePoint last_screenshot_on_;

    bool is_ready_;
};