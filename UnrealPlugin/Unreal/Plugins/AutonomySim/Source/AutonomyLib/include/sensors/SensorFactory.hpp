#ifndef autonomylib_sensors_SensorFactoryBase_hpp
#define autonomylib_sensors_SensorFactoryBase_hpp

#include "SensorBase.hpp"
#include "SensorCollection.hpp"
#include <memory>

// sensors
#include "sensors/barometer/BarometerSimple.hpp"
#include "sensors/gps/GpsSimple.hpp"
#include "sensors/imu/ImuSimple.hpp"
#include "sensors/magnetometer/MagnetometerSimple.hpp"

namespace nervosys {
namespace autonomylib {

class SensorFactory {
  public:
    // creates one sensor from settings
    virtual std::shared_ptr<SensorBase>
    createSensorFromSettings(const AutonomySimSettings::SensorSetting *sensor_setting) const {
        switch (sensor_setting->sensor_type) {
        case SensorBase::SensorType::Imu:
            return std::shared_ptr<ImuSimple>(
                new ImuSimple(*static_cast<const AutonomySimSettings::ImuSetting *>(sensor_setting)));
        case SensorBase::SensorType::Magnetometer:
            return std::shared_ptr<MagnetometerSimple>(
                new MagnetometerSimple(*static_cast<const AutonomySimSettings::MagnetometerSetting *>(sensor_setting)));
        case SensorBase::SensorType::Gps:
            return std::shared_ptr<GpsSimple>(
                new GpsSimple(*static_cast<const AutonomySimSettings::GpsSetting *>(sensor_setting)));
        case SensorBase::SensorType::Barometer:
            return std::shared_ptr<BarometerSimple>(
                new BarometerSimple(*static_cast<const AutonomySimSettings::BarometerSetting *>(sensor_setting)));
        default:
            throw new std::invalid_argument("Unexpected sensor type");
        }
    }

    // creates sensor-collection
    virtual void createSensorsFromSettings(
        const std::map<std::string, std::shared_ptr<AutonomySimSettings::SensorSetting>> &sensors_settings,
        SensorCollection &sensors, vector<shared_ptr<SensorBase>> &sensor_storage) const {
        for (const auto &sensor_setting_pair : sensors_settings) {
            const AutonomySimSettings::SensorSetting *sensor_setting = sensor_setting_pair.second.get();

            // ignore sensors that are marked "disabled" in settings
            if (sensor_setting == nullptr || !sensor_setting->enabled)
                continue;

            std::shared_ptr<SensorBase> sensor = createSensorFromSettings(sensor_setting);
            if (sensor) {
                sensor_storage.push_back(sensor);
                sensors.insert(sensor.get(), sensor_setting->sensor_type);
            }
        }
    }

    virtual ~SensorFactory() = default;
};

} // namespace autonomylib
} // namespace nervosys

#endif