#ifndef autonomylib_modules_communication_RTXRFPropagation_hpp
#define autonomylib_modules_communication_RTXRFPropagation_hpp

#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace autonomysim {

/**
 * @brief RTX-accelerated RF propagation simulator using ray tracing
 *
 * Utilizes NVIDIA RTX GPUs for real-time radio frequency propagation modeling
 * in complex 3D environments. Simulates path loss, signal extinction, multipath,
 * diffraction, and atmospheric effects.
 *
 * Features:
 * - Hardware ray tracing (RT cores) for RF path calculation
 * - Multi-frequency support (UHF to mmWave)
 * - Real-time signal strength prediction
 * - Line-of-sight (LOS) determination
 * - Non-line-of-sight (NLOS) multipath modeling
 * - Material-specific attenuation (concrete, metal, vegetation)
 * - Weather effects (rain, fog, snow)
 * - Terrain effects (ground reflection, diffraction)
 *
 * Use Cases:
 * - Drone-to-drone communication link quality
 * - Ground control to UAV signal strength
 * - Jamming and interference simulation
 * - Network planning and coverage analysis
 * - Autonomous swarm connectivity validation
 */
class RTXRFPropagation {
  public:
    // RF band classifications
    enum class RFBand {
        VHF_30_300MHz,   // 30-300 MHz (long range, high diffraction)
        UHF_300MHz_3GHz, // 300 MHz - 3 GHz (TV, cellular, WiFi 2.4G)
        SHF_3_30GHz,     // 3-30 GHz (WiFi 5G, radar, satellite)
        EHF_30_300GHz,   // 30-300 GHz (mmWave 5G, point-to-point)
        Custom           // User-defined frequency
    };

    // Material types with RF properties
    enum class MaterialType {
        Air,        // No attenuation
        Concrete,   // Buildings, bunkers (high attenuation)
        Metal,      // Vehicles, structures (reflection, blocking)
        Wood,       // Trees, structures (moderate attenuation)
        Brick,      // Buildings (high attenuation)
        Glass,      // Windows (low attenuation, reflection)
        Vegetation, // Trees, foliage (frequency-dependent)
        Water,      // Rain, bodies of water (frequency-dependent)
        Soil,       // Ground terrain (absorption)
        Asphalt,    // Roads (low attenuation)
        Custom      // User-defined properties
    };

    // Weather conditions affecting RF propagation
    enum class WeatherCondition {
        Clear,        // No atmospheric effects
        LightRain,    // <2.5 mm/hr (minor attenuation at high freq)
        ModerateRain, // 2.5-10 mm/hr (noticeable attenuation)
        HeavyRain,    // >10 mm/hr (significant attenuation)
        Fog,          // Visibility < 1km (scattering)
        Snow,         // Frozen precipitation (less attenuation than rain)
        Storm         // Combined effects (severe attenuation)
    };

    // RF propagation model type
    enum class PropagationModel {
        FreeSpace,       // Line-of-sight only (Friis equation)
        TwoRay,          // Ground reflection included
        Longley_Rice,    // Irregular terrain (ITM model)
        Okumura_Hata,    // Urban/suburban empirical model
        COST231_Hata,    // Extended Hata for higher frequencies
        Walfish_Ikegami, // Urban street canyon model
        RTX_RayTracing   // Full RTX-accelerated ray tracing (RECOMMENDED)
    };

    // RF transmitter configuration
    struct Transmitter {
        std::string id;             // Unique identifier
        Vector3r position;          // 3D position (meters)
        real_T frequency_hz;        // Carrier frequency (Hz)
        real_T power_dbm;           // Transmit power (dBm)
        real_T antenna_gain_dbi;    // Antenna gain (dBi)
        real_T antenna_height_m;    // Height above ground (m)
        Vector3r antenna_direction; // Directional antenna pointing
        real_T beamwidth_degrees;   // Antenna beamwidth (degrees, 360 = omnidirectional)
        RFBand band;                // Frequency band
        bool active;                // Transmitter enabled

        Transmitter()
            : frequency_hz(2.4e9), power_dbm(20.0), antenna_gain_dbi(2.0), antenna_height_m(1.5),
              antenna_direction(1, 0, 0), beamwidth_degrees(360.0), band(RFBand::UHF_300MHz_3GHz), active(true) {}
    };

    // RF receiver configuration
    struct Receiver {
        std::string id;          // Unique identifier
        Vector3r position;       // 3D position (meters)
        real_T sensitivity_dbm;  // Receiver sensitivity (dBm)
        real_T antenna_gain_dbi; // Antenna gain (dBi)
        real_T antenna_height_m; // Height above ground (m)
        real_T noise_figure_db;  // Receiver noise figure (dB)

        Receiver() : sensitivity_dbm(-90.0), antenna_gain_dbi(2.0), antenna_height_m(1.5), noise_figure_db(6.0) {}
    };

    // RF propagation path result
    struct PropagationResult {
        bool line_of_sight;             // Direct LOS path exists
        real_T path_loss_db;            // Total path loss (dB)
        real_T received_power_dbm;      // Received signal strength (dBm)
        real_T signal_to_noise_db;      // SNR (dB)
        real_T distance_m;              // Transmitter-receiver distance (m)
        real_T fresnel_clearance;       // Fresnel zone clearance (0-1, >0.6 = good)
        uint32_t num_reflections;       // Number of multipath reflections
        uint32_t num_diffractions;      // Number of diffractions over obstacles
        real_T delay_spread_ns;         // Multipath delay spread (nanoseconds)
        real_T doppler_shift_hz;        // Doppler shift due to motion (Hz)
        std::vector<Vector3r> ray_path; // Ray path points for visualization
        std::string status;             // Result status message

        PropagationResult()
            : line_of_sight(false), path_loss_db(1000.0) // Large number = no signal
              ,
              received_power_dbm(-200.0), signal_to_noise_db(-100.0), distance_m(0.0), fresnel_clearance(0.0),
              num_reflections(0), num_diffractions(0), delay_spread_ns(0.0), doppler_shift_hz(0.0),
              status("Not computed") {}
    };

    // Material RF properties
    struct MaterialProperties {
        MaterialType type;
        real_T relative_permittivity;  // Dielectric constant (εr)
        real_T conductivity_s_m;       // Electrical conductivity (S/m)
        real_T attenuation_db_per_m;   // Attenuation coefficient (dB/m)
        real_T reflection_coefficient; // Surface reflection (0-1)
        real_T roughness_m;            // Surface roughness (meters)

        MaterialProperties()
            : type(MaterialType::Air), relative_permittivity(1.0), conductivity_s_m(0.0), attenuation_db_per_m(0.0),
              reflection_coefficient(0.0), roughness_m(0.0) {}
    };

    // RTX configuration
    struct RTXConfig {
        bool enable_rtx;               // Use RTX hardware acceleration
        uint32_t max_ray_bounces;      // Maximum reflections (1-10)
        uint32_t rays_per_query;       // Rays per propagation query (quality vs speed)
        bool enable_diffraction;       // Calculate diffraction over obstacles
        bool enable_ground_reflection; // Include ground plane reflection
        bool enable_atmospheric;       // Atmospheric/weather effects
        PropagationModel model;        // Propagation model to use
        WeatherCondition weather;      // Current weather condition
        real_T ambient_temperature_c;  // Ambient temperature (Celsius)
        real_T humidity_percent;       // Relative humidity (%)

        RTXConfig()
            : enable_rtx(true), max_ray_bounces(3), rays_per_query(64), enable_diffraction(true),
              enable_ground_reflection(true), enable_atmospheric(true), model(PropagationModel::RTX_RayTracing),
              weather(WeatherCondition::Clear), ambient_temperature_c(20.0), humidity_percent(50.0) {}
    };

  public:
    RTXRFPropagation();
    ~RTXRFPropagation();

    // Initialization
    bool initialize(const RTXConfig &config);
    void shutdown();
    bool isInitialized() const { return initialized_; }

    // Configuration
    void setConfig(const RTXConfig &config);
    RTXConfig getConfig() const;
    void setWeather(WeatherCondition weather);
    void setPropagationModel(PropagationModel model);

    // Material properties
    void registerMaterial(const std::string &material_id, const MaterialProperties &props);
    MaterialProperties getMaterial(const std::string &material_id) const;
    void setDefaultMaterial(MaterialType type);

    // Transmitter/Receiver management
    void registerTransmitter(const Transmitter &tx);
    void registerReceiver(const Receiver &rx);
    void updateTransmitter(const std::string &tx_id, const Vector3r &position);
    void updateReceiver(const std::string &rx_id, const Vector3r &position);
    void removeTransmitter(const std::string &tx_id);
    void removeReceiver(const std::string &rx_id);

    // RF propagation queries
    PropagationResult computePropagation(const std::string &tx_id, const std::string &rx_id);
    PropagationResult computePropagation(const Transmitter &tx, const Receiver &rx);
    std::vector<PropagationResult> computeAllLinks(); // All TX-RX pairs

    // Advanced queries
    bool hasLineOfSight(const Vector3r &tx_pos, const Vector3r &rx_pos) const;
    real_T estimatePathLoss(const Vector3r &tx_pos, const Vector3r &rx_pos, real_T frequency_hz);
    real_T computeReceivedPower(const Transmitter &tx, const Receiver &rx);
    real_T computeLinkMargin(const Transmitter &tx, const Receiver &rx); // dB above sensitivity

    // Coverage analysis
    std::vector<Vector3r> computeCoverageArea(const std::string &tx_id, real_T min_rssi_dbm, real_T grid_resolution_m);
    real_T computeOutageProbability(const std::string &tx_id, const std::string &rx_id, real_T required_snr_db);

    // Interference and jamming
    void registerJammer(const Transmitter &jammer);
    real_T computeInterference(const Receiver &rx, real_T signal_frequency_hz);
    real_T computeSINR(const std::string &tx_id, const std::string &rx_id); // Signal-to-interference-plus-noise

    // Doppler and mobility
    void setTransmitterVelocity(const std::string &tx_id, const Vector3r &velocity_m_s);
    void setReceiverVelocity(const std::string &rx_id, const Vector3r &velocity_m_s);
    real_T computeDopplerShift(const Transmitter &tx, const Receiver &rx, const Vector3r &tx_velocity,
                               const Vector3r &rx_velocity);

    // Environment/scene management
    void setScene(void *unreal_scene_ptr); // Pointer to Unreal scene geometry
    void updateSceneGeometry();
    void setTerrainElevation(const std::vector<std::vector<real_T>> &elevation_grid_m);

    // Visualization support
    std::vector<Vector3r> getRayPath(const std::string &tx_id, const std::string &rx_id);
    void enableVisualization(bool enable) { visualization_enabled_ = enable; }

    // Performance monitoring
    real_T getAverageComputeTimeMs() const;
    uint32_t getRTXCoreUtilization() const; // Percentage
    void resetStatistics();

  private:
    // Helper functions
    PropagationResult computeRTXRayTracing(const Transmitter &tx, const Receiver &rx);
    PropagationResult computeFreeSpace(const Transmitter &tx, const Receiver &rx);
    PropagationResult computeTwoRay(const Transmitter &tx, const Receiver &rx);
    PropagationResult computeOkumuraHata(const Transmitter &tx, const Receiver &rx);

    real_T computeFreeSpacePathLoss(real_T distance_m, real_T frequency_hz) const;
    real_T computeAtmosphericLoss(real_T distance_m, real_T frequency_hz, WeatherCondition weather) const;
    real_T computeMaterialPenetrationLoss(const MaterialProperties &material, real_T thickness_m,
                                          real_T frequency_hz) const;
    real_T computeFresnelZone(real_T distance_m, real_T frequency_hz, uint32_t zone_number) const;

    bool checkRTXAvailable() const;
    void initializeRTXContext();
    void buildRTXScene();

  private:
    bool initialized_;
    RTXConfig config_;

    // Transmitters and receivers
    std::map<std::string, Transmitter> transmitters_;
    std::map<std::string, Receiver> receivers_;
    std::map<std::string, Vector3r> transmitter_velocities_;
    std::map<std::string, Vector3r> receiver_velocities_;

    // Materials
    std::map<std::string, MaterialProperties> materials_;
    MaterialType default_material_type_;

    // Jammers (special transmitters for interference)
    std::vector<Transmitter> jammers_;

    // Scene geometry
    void *scene_ptr_;
    std::vector<std::vector<real_T>> terrain_elevation_;

    // RTX context (OptiX handles)
    void *optix_context_;
    void *optix_pipeline_;
    void *optix_sbt_; // Shader binding table

    // Visualization
    bool visualization_enabled_;
    std::map<std::pair<std::string, std::string>, std::vector<Vector3r>> cached_ray_paths_;

    // Performance tracking
    mutable real_T total_compute_time_ms_;
    mutable uint32_t num_queries_;

    // Thread safety
    mutable std::mutex mutex_;
};

} // namespace autonomysim

#endif // autonomylib_modules_communication_RTXRFPropagation_hpp
