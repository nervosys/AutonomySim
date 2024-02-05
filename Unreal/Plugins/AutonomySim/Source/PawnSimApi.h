#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Particles/ParticleSystemComponent.h"
#include "UnrealImageCapture.h"

#include "NedTransform.h"
#include "PIPCamera.h"
#include "SimJoyStick/SimJoyStick.h"
#include "api/VehicleApiBase.hpp"
#include "api/VehicleSimApiBase.hpp"
#include "common/AutonomySimSettings.hpp"
#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include "common/GeodeticConverter.hpp"
#include "common/common_utils/Signal.hpp"
#include "common/common_utils/UniqueValueMap.hpp"
#include "physics/Kinematics.hpp"
#include <memory>
#include <vector>

#include "PawnEvents.h"

class PawnSimApi : public nervosys::autonomylib::VehicleSimApiBase {
  public: // types
    typedef nervosys::autonomylib::GeoPoint GeoPoint;
    typedef nervosys::autonomylib::Vector2r Vector2r;
    typedef nervosys::autonomylib::Vector3r Vector3r;
    typedef nervosys::autonomylib::Pose Pose;
    typedef nervosys::autonomylib::Quaternionr Quaternionr;
    typedef nervosys::autonomylib::CollisionInfo CollisionInfo;
    typedef nervosys::autonomylib::VectorMath VectorMath;
    typedef nervosys::autonomylib::real_T real_T;
    typedef nervosys::autonomylib::Utils Utils;
    typedef nervosys::autonomylib::AutonomySimSettings::VehicleSetting VehicleSetting;
    typedef nervosys::autonomylib::ImageCaptureBase ImageCaptureBase;
    typedef nervosys::autonomylib::DetectionInfo DetectionInfo;
    typedef nervosys::autonomylib::Kinematics Kinematics;

    struct Params {
        APawn *pawn;
        const NedTransform *global_transform;
        PawnEvents *pawn_events;
        common_utils::UniqueValueMap<std::string, APIPCamera *> cameras;
        UClass *pip_camera_class;
        UParticleSystem *collision_display_template;
        nervosys::autonomylib::GeoPoint home_geopoint;
        std::string vehicle_name;

        Params() {}

        Params(APawn *pawn_val, const NedTransform *global_transform_val, PawnEvents *pawn_events_val,
               const common_utils::UniqueValueMap<std::string, APIPCamera *> &cameras_val, UClass *pip_camera_class_val,
               UParticleSystem *collision_display_template_val, const nervosys::autonomylib::GeoPoint &home_geopoint_val,
               const std::string &vehicle_name_val)
            : pawn(pawn_val), global_transform(global_transform_val), pawn_events(pawn_events_val),
              cameras(cameras_val), pip_camera_class(pip_camera_class_val),
              collision_display_template(collision_display_template_val), home_geopoint(home_geopoint_val),
              vehicle_name(vehicle_name_val) {}
    };

  public: // implementation of VehicleSimApiBase
    virtual void initialize() override;

    virtual void resetImplementation() override;
    virtual void update() override;

    virtual const UnrealImageCapture *getImageCapture() const override;
    virtual Pose getPose() const override;
    virtual void setPose(const Pose &pose, bool ignore_collision) override;

    virtual CollisionInfo getCollisionInfo() const override;
    virtual CollisionInfo getCollisionInfoAndReset() override;
    virtual int getRemoteControlID() const override;
    virtual nervosys::autonomylib::RCData getRCData() const override;
    virtual std::string getVehicleName() const override { return params_.vehicle_name; }
    virtual void toggleTrace() override;
    virtual void setTraceLine(const std::vector<float> &color_rgba, float thickness) override;

    virtual void updateRenderedState(float dt) override;
    virtual void updateRendering(float dt) override;
    virtual const nervosys::autonomylib::Kinematics::State *getGroundTruthKinematics() const override;
    virtual void setKinematics(const nervosys::autonomylib::Kinematics::State &state, bool ignore_collision) override;
    virtual const nervosys::autonomylib::Environment *getGroundTruthEnvironment() const override;
    virtual std::string getRecordFileLine(bool is_header_line) const override;
    virtual void reportState(nervosys::autonomylib::StateReporter &reporter) override;

  protected: // additional interface for derived class
    virtual void pawnTick(float dt);
    void setPoseInternal(const Pose &pose, bool ignore_collision);
    virtual nervosys::autonomylib::VehicleApiBase *getVehicleApiBase() const;
    nervosys::autonomylib::Kinematics *getKinematics();
    nervosys::autonomylib::Environment *getEnvironment();

  public: // Unreal specific methods
    PawnSimApi(const Params &params);

    // returns one of the cameras attached to the pawn
    const APIPCamera *getCamera(const std::string &camera_name) const;
    APIPCamera *getCamera(const std::string &camera_name);
    int getCameraCount();

    virtual bool testLineOfSightToPoint(const nervosys::autonomylib::GeoPoint &point) const;

    // if enabled, this would show some flares
    void displayCollisionEffect(FVector hit_location, const FHitResult &hit);

    // return the attached pawn
    APawn *getPawn();

    // get/set pose
    // parameters in NED frame
    void setDebugPose(const Pose &debug_pose);

    FVector getUUPosition() const;
    FRotator getUUOrientation() const;

    const NedTransform &getNedTransform() const;

    void possess();
    void setRCForceFeedback(float rumble_strength, float auto_center);

  private: // methods
    bool canTeleportWhileMove() const;
    void allowPassthroughToggleInput();
    void detectUsbRc();
    void setupCamerasFromSettings(const common_utils::UniqueValueMap<std::string, APIPCamera *> &cameras);
    void createCamerasFromSettings();
    // on collision, pawns should update this
    void onCollision(class UPrimitiveComponent *MyComp, class AActor *Other, class UPrimitiveComponent *OtherComp,
                     bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse,
                     const FHitResult &Hit);

    // these methods are for future usage
    void plot(std::istream &s, FColor color, const Vector3r &offset);
    PawnSimApi::Pose toPose(const FVector &u_position, const FQuat &u_quat) const;
    void updateKinematics(float dt);
    void setStartPosition(const FVector &position, const FRotator &rotator);

  private: // vars
    typedef nervosys::autonomylib::AutonomySimSettings AutonomySimSettings;
    typedef nervosys::autonomylib::Environment Environment;

    Params params_;
    common_utils::UniqueValueMap<std::string, APIPCamera *> cameras_;
    nervosys::autonomylib::GeoPoint home_geo_point_;

    std::string vehicle_name_;
    NedTransform ned_transform_;

    FVector ground_trace_end_;
    FVector ground_margin_;
    std::unique_ptr<UnrealImageCapture> image_capture_;
    std::string log_line_;

    mutable nervosys::autonomylib::RCData rc_data_;
    mutable SimJoyStick joystick_;
    mutable SimJoyStick::State joystick_state_;

    struct State {
        FVector start_location;
        FRotator start_rotation;
        FVector last_position;
        FVector last_debug_position;
        FVector current_position;
        FVector current_debug_position;
        FVector debug_position_offset;
        bool tracing_enabled;
        bool collisions_enabled;
        bool passthrough_enabled;
        bool was_last_move_teleport;
        CollisionInfo collision_info;

        FVector mesh_origin;
        FVector mesh_bounds;
        FVector ground_offset;
        FVector transformation_offset;
    };

    State state_, initial_state_;

    std::unique_ptr<nervosys::autonomylib::Kinematics> kinematics_;
    std::unique_ptr<nervosys::autonomylib::Environment> environment_;

    FColor trace_color_ = FColor::Purple;
    float trace_thickness_ = 3.0f;
};
