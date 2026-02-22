//! Sensor data types and processing

use crate::backend::{Position, Rotation, Vec3};
use serde::{Deserialize, Serialize};

/// IMU (Inertial Measurement Unit) data
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ImuData {
    pub timestamp: f64,
    pub linear_acceleration: Vec3,
    pub angular_velocity: Vec3,
    pub orientation: Rotation,
}

/// GPS data
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GpsData {
    pub timestamp: f64,
    pub latitude: f64,
    pub longitude: f64,
    pub altitude: f64,
    pub velocity: Vec3,
    pub eph: f64, // Horizontal position error
    pub epv: f64, // Vertical position error
    pub fix_type: GpsFixType,
}

/// GPS fix type
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum GpsFixType {
    NoFix = 0,
    Fix2D = 2,
    Fix3D = 3,
    DGps = 4,
    RtkFloat = 5,
    RtkFixed = 6,
}

/// Magnetometer data
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MagnetometerData {
    pub timestamp: f64,
    pub magnetic_field: Vec3, // Tesla
}

/// Barometer data
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BarometerData {
    pub timestamp: f64,
    pub pressure: f64,    // Pascal
    pub altitude: f64,    // Meters
    pub temperature: f64, // Celsius
}

/// Distance sensor data
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DistanceSensorData {
    pub timestamp: f64,
    pub distance: f64, // Meters
    pub min_range: f64,
    pub max_range: f64,
}

/// LiDAR point
#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub struct LidarPoint {
    pub position: Position,
    pub intensity: f32,
}

/// LiDAR data
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LidarData {
    pub timestamp: f64,
    pub points: Vec<LidarPoint>,
    pub pose: Position,
}

/// Camera image data
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CameraData {
    pub timestamp: f64,
    pub width: u32,
    pub height: u32,
    pub image_type: ImageType,
    pub pixels: Vec<u8>,
    pub camera_info: CameraInfo,
}

/// Camera information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CameraInfo {
    pub projection_matrix: [f64; 16],
    pub fov: f64, // Field of view in degrees
}

/// Image type
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum ImageType {
    /// RGB color image
    Rgb,
    /// RGBA with alpha
    Rgba,
    /// Grayscale
    Gray,
    /// Depth map (16-bit or 32-bit float)
    Depth,
    /// Segmentation mask
    Segmentation,
}

/// RF antenna measurement
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RfAntennaData {
    pub timestamp: f64,
    pub frequency_hz: f64,
    pub rssi_dbm: f64,         // Received Signal Strength Indicator
    pub snr_db: f64,           // Signal-to-Noise Ratio
    pub phase: f64,            // Phase in radians
    pub doppler_shift_hz: f64, // Doppler shift
}

/// Generic sensor data container
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum SensorData {
    Imu(ImuData),
    Gps(GpsData),
    Magnetometer(MagnetometerData),
    Barometer(BarometerData),
    Distance(DistanceSensorData),
    Lidar(LidarData),
    Camera(CameraData),
    RfAntenna(RfAntennaData),
}

impl SensorData {
    pub fn timestamp(&self) -> f64 {
        match self {
            SensorData::Imu(d) => d.timestamp,
            SensorData::Gps(d) => d.timestamp,
            SensorData::Magnetometer(d) => d.timestamp,
            SensorData::Barometer(d) => d.timestamp,
            SensorData::Distance(d) => d.timestamp,
            SensorData::Lidar(d) => d.timestamp,
            SensorData::Camera(d) => d.timestamp,
            SensorData::RfAntenna(d) => d.timestamp,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use nalgebra::{UnitQuaternion, Vector3};

    #[test]
    fn test_imu_data() {
        let imu = ImuData {
            timestamp: 1.0,
            linear_acceleration: Vector3::new(0.0, 0.0, 9.81),
            angular_velocity: Vector3::zeros(),
            orientation: UnitQuaternion::identity(),
        };

        assert_eq!(imu.timestamp, 1.0);
        assert_eq!(imu.linear_acceleration.z, 9.81);
    }

    #[test]
    fn test_gps_fix_type() {
        let gps = GpsData {
            timestamp: 1.0,
            latitude: 47.6,
            longitude: -122.3,
            altitude: 100.0,
            velocity: Vector3::zeros(),
            eph: 1.0,
            epv: 2.0,
            fix_type: GpsFixType::Fix3D,
        };

        assert_eq!(gps.fix_type, GpsFixType::Fix3D);
    }

    #[test]
    fn test_sensor_data_timestamp() {
        let imu = ImuData {
            timestamp: 5.5,
            linear_acceleration: Vector3::zeros(),
            angular_velocity: Vector3::zeros(),
            orientation: UnitQuaternion::identity(),
        };

        let sensor_data = SensorData::Imu(imu);
        assert_eq!(sensor_data.timestamp(), 5.5);
    }
}
