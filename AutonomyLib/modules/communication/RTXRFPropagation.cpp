#include "RTXRFPropagation.hpp"
#include "common/common_utils/Utils.hpp"
#include <algorithm>
#include <cmath>

namespace autonomysim {

// Physical constants
static const real_T SPEED_OF_LIGHT = 299792458.0;        // m/s
static const real_T BOLTZMANN_CONSTANT = 1.38064852e-23; // J/K
static const real_T PI = 3.14159265358979323846;

RTXRFPropagation::RTXRFPropagation()
    : initialized_(false), scene_ptr_(nullptr), optix_context_(nullptr), optix_pipeline_(nullptr), optix_sbt_(nullptr),
      visualization_enabled_(false), total_compute_time_ms_(0.0), num_queries_(0),
      default_material_type_(MaterialType::Air) {
    // Initialize default material properties
    MaterialProperties air;
    air.type = MaterialType::Air;
    air.relative_permittivity = 1.0;
    air.conductivity_s_m = 0.0;
    air.attenuation_db_per_m = 0.0;
    air.reflection_coefficient = 0.0;
    air.roughness_m = 0.0;
    materials_["air"] = air;

    MaterialProperties concrete;
    concrete.type = MaterialType::Concrete;
    concrete.relative_permittivity = 5.0;
    concrete.conductivity_s_m = 0.01;
    concrete.attenuation_db_per_m = 15.0;
    concrete.reflection_coefficient = 0.3;
    concrete.roughness_m = 0.01;
    materials_["concrete"] = concrete;

    MaterialProperties metal;
    metal.type = MaterialType::Metal;
    metal.relative_permittivity = 1.0;
    metal.conductivity_s_m = 1e7;
    metal.attenuation_db_per_m = 100.0; // Nearly complete blocking
    metal.reflection_coefficient = 0.95;
    metal.roughness_m = 0.001;
    materials_["metal"] = metal;

    MaterialProperties wood;
    wood.type = MaterialType::Wood;
    wood.relative_permittivity = 2.5;
    wood.conductivity_s_m = 0.001;
    wood.attenuation_db_per_m = 5.0;
    wood.reflection_coefficient = 0.1;
    wood.roughness_m = 0.005;
    materials_["wood"] = wood;

    MaterialProperties vegetation;
    vegetation.type = MaterialType::Vegetation;
    vegetation.relative_permittivity = 10.0;
    vegetation.conductivity_s_m = 0.1;
    vegetation.attenuation_db_per_m = 0.5; // Frequency dependent in practice
    vegetation.reflection_coefficient = 0.05;
    vegetation.roughness_m = 0.1;
    materials_["vegetation"] = vegetation;

    MaterialProperties glass;
    glass.type = MaterialType::Glass;
    glass.relative_permittivity = 6.0;
    glass.conductivity_s_m = 1e-12;
    glass.attenuation_db_per_m = 2.0;
    glass.reflection_coefficient = 0.2;
    glass.roughness_m = 0.0001;
    materials_["glass"] = glass;
}

RTXRFPropagation::~RTXRFPropagation() { shutdown(); }

bool RTXRFPropagation::initialize(const RTXConfig &config) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_) {
        return true;
    }

    config_ = config;

    // Check if RTX is available
    if (config_.enable_rtx) {
        if (!checkRTXAvailable()) {
            Utils::log("RTX hardware not available, falling back to analytical models", Utils::kLogLevelWarn);
            config_.enable_rtx = false;
        } else {
            Utils::log("Initializing NVIDIA RTX for RF propagation simulation", Utils::kLogLevelInfo);
            initializeRTXContext();
        }
    }

    initialized_ = true;
    Utils::log("RTXRFPropagation initialized successfully", Utils::kLogLevelInfo);
    return true;
}

void RTXRFPropagation::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        return;
    }

    // Cleanup RTX resources
    if (optix_context_ != nullptr) {
        // TODO: Cleanup OptiX context, pipeline, SBT
        optix_context_ = nullptr;
        optix_pipeline_ = nullptr;
        optix_sbt_ = nullptr;
    }

    transmitters_.clear();
    receivers_.clear();
    jammers_.clear();
    cached_ray_paths_.clear();

    initialized_ = false;
}

void RTXRFPropagation::setConfig(const RTXConfig &config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
}

RTXRFPropagation::RTXConfig RTXRFPropagation::getConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

void RTXRFPropagation::setWeather(WeatherCondition weather) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.weather = weather;
}

void RTXRFPropagation::setPropagationModel(PropagationModel model) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.model = model;
}

void RTXRFPropagation::registerMaterial(const std::string &material_id, const MaterialProperties &props) {
    std::lock_guard<std::mutex> lock(mutex_);
    materials_[material_id] = props;
}

RTXRFPropagation::MaterialProperties RTXRFPropagation::getMaterial(const std::string &material_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = materials_.find(material_id);
    if (it != materials_.end()) {
        return it->second;
    }
    return materials_.at("air"); // Default
}

void RTXRFPropagation::setDefaultMaterial(MaterialType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    default_material_type_ = type;
}

void RTXRFPropagation::registerTransmitter(const Transmitter &tx) {
    std::lock_guard<std::mutex> lock(mutex_);
    transmitters_[tx.id] = tx;
}

void RTXRFPropagation::registerReceiver(const Receiver &rx) {
    std::lock_guard<std::mutex> lock(mutex_);
    receivers_[rx.id] = rx;
}

void RTXRFPropagation::updateTransmitter(const std::string &tx_id, const Vector3r &position) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = transmitters_.find(tx_id);
    if (it != transmitters_.end()) {
        it->second.position = position;
    }
}

void RTXRFPropagation::updateReceiver(const std::string &rx_id, const Vector3r &position) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = receivers_.find(rx_id);
    if (it != receivers_.end()) {
        it->second.position = position;
    }
}

void RTXRFPropagation::removeTransmitter(const std::string &tx_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    transmitters_.erase(tx_id);
}

void RTXRFPropagation::removeReceiver(const std::string &rx_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    receivers_.erase(rx_id);
}

RTXRFPropagation::PropagationResult RTXRFPropagation::computePropagation(const std::string &tx_id,
                                                                         const std::string &rx_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto tx_it = transmitters_.find(tx_id);
    auto rx_it = receivers_.find(rx_id);

    if (tx_it == transmitters_.end() || rx_it == receivers_.end()) {
        PropagationResult result;
        result.status = "Transmitter or receiver not found";
        return result;
    }

    return computePropagation(tx_it->second, rx_it->second);
}

RTXRFPropagation::PropagationResult RTXRFPropagation::computePropagation(const Transmitter &tx, const Receiver &rx) {
    auto start_time = std::chrono::high_resolution_clock::now();

    PropagationResult result;

    // Select propagation model
    switch (config_.model) {
    case PropagationModel::RTX_RayTracing:
        if (config_.enable_rtx && optix_context_ != nullptr) {
            result = computeRTXRayTracing(tx, rx);
        } else {
            // Fallback to free space if RTX not available
            result = computeFreeSpace(tx, rx);
        }
        break;
    case PropagationModel::FreeSpace:
        result = computeFreeSpace(tx, rx);
        break;
    case PropagationModel::TwoRay:
        result = computeTwoRay(tx, rx);
        break;
    case PropagationModel::Okumura_Hata:
    case PropagationModel::COST231_Hata:
        result = computeOkumuraHata(tx, rx);
        break;
    default:
        result = computeFreeSpace(tx, rx);
        break;
    }

    // Add atmospheric/weather losses
    if (config_.enable_atmospheric) {
        real_T atmo_loss = computeAtmosphericLoss(result.distance_m, tx.frequency_hz, config_.weather);
        result.path_loss_db += atmo_loss;
        result.received_power_dbm -= atmo_loss;
    }

    // Compute SNR
    real_T noise_power_dbm = -174.0 + 10.0 * std::log10(tx.frequency_hz) + rx.noise_figure_db;
    result.signal_to_noise_db = result.received_power_dbm - noise_power_dbm;

    result.status = "Success";

    // Performance tracking
    auto end_time = std::chrono::high_resolution_clock::now();
    real_T compute_time = std::chrono::duration<real_T, std::milli>(end_time - start_time).count();
    total_compute_time_ms_ += compute_time;
    num_queries_++;

    return result;
}

std::vector<RTXRFPropagation::PropagationResult> RTXRFPropagation::computeAllLinks() {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<PropagationResult> results;

    for (const auto &tx_pair : transmitters_) {
        for (const auto &rx_pair : receivers_) {
            if (tx_pair.second.active) {
                PropagationResult result = computePropagation(tx_pair.second, rx_pair.second);
                results.push_back(result);
            }
        }
    }

    return results;
}

bool RTXRFPropagation::hasLineOfSight(const Vector3r &tx_pos, const Vector3r &rx_pos) const {
    // Simple implementation - would use RTX ray tracing in full version
    // For now, just check if there's a direct path

    if (optix_context_ != nullptr) {
        // TODO: Cast ray using OptiX, return true if no intersection
        return true; // Placeholder
    }

    // Fallback: assume LOS if within certain distance
    real_T distance = (rx_pos - tx_pos).norm();
    return distance < 1000.0; // 1km LOS assumption
}

real_T RTXRFPropagation::estimatePathLoss(const Vector3r &tx_pos, const Vector3r &rx_pos, real_T frequency_hz) {
    real_T distance = (rx_pos - tx_pos).norm();
    return computeFreeSpacePathLoss(distance, frequency_hz);
}

real_T RTXRFPropagation::computeReceivedPower(const Transmitter &tx, const Receiver &rx) {
    PropagationResult result = computePropagation(tx, rx);
    return result.received_power_dbm;
}

real_T RTXRFPropagation::computeLinkMargin(const Transmitter &tx, const Receiver &rx) {
    real_T rx_power = computeReceivedPower(tx, rx);
    return rx_power - rx.sensitivity_dbm;
}

// Private helper functions

RTXRFPropagation::PropagationResult RTXRFPropagation::computeRTXRayTracing(const Transmitter &tx, const Receiver &rx) {
    PropagationResult result;

    // TODO: Full RTX ray tracing implementation using OptiX
    // This would:
    // 1. Launch rays from TX to RX
    // 2. Trace reflections/diffractions using RT cores
    // 3. Accumulate path loss from each interaction
    // 4. Return combined result

    // For now, fallback to analytical
    result = computeTwoRay(tx, rx);
    result.status = "RTX ray tracing (placeholder - using two-ray model)";

    return result;
}

RTXRFPropagation::PropagationResult RTXRFPropagation::computeFreeSpace(const Transmitter &tx, const Receiver &rx) {
    PropagationResult result;

    // Calculate distance
    result.distance_m = (rx.position - tx.position).norm();

    // Free space path loss (Friis equation)
    result.path_loss_db = computeFreeSpacePathLoss(result.distance_m, tx.frequency_hz);

    // Received power
    result.received_power_dbm = tx.power_dbm + tx.antenna_gain_dbi + rx.antenna_gain_dbi - result.path_loss_db;

    // Assume LOS for free space
    result.line_of_sight = true;
    result.fresnel_clearance = 1.0;
    result.num_reflections = 0;
    result.num_diffractions = 0;

    // Store ray path for visualization
    if (visualization_enabled_) {
        result.ray_path.push_back(tx.position);
        result.ray_path.push_back(rx.position);
    }

    result.status = "Free space model";
    return result;
}

RTXRFPropagation::PropagationResult RTXRFPropagation::computeTwoRay(const Transmitter &tx, const Receiver &rx) {
    PropagationResult result = computeFreeSpace(tx, rx);

    // Add ground reflection component
    if (config_.enable_ground_reflection) {
        real_T h_tx = tx.antenna_height_m;
        real_T h_rx = rx.antenna_height_m;
        real_T distance = result.distance_m;

        // Ground reflection path length
        real_T d_reflected = std::sqrt(distance * distance + (h_tx + h_rx) * (h_tx + h_rx));
        real_T path_loss_reflected = computeFreeSpacePathLoss(d_reflected, tx.frequency_hz);

        // Ground reflection coefficient (simplified)
        real_T ground_reflection = -0.7; // Typical for ground

        // Combine direct and reflected paths (simplified)
        real_T additional_loss = -10.0 * std::log10(1.0 + std::pow(10.0, ground_reflection / 10.0));
        result.path_loss_db += additional_loss;
        result.received_power_dbm -= additional_loss;

        result.num_reflections = 1;
        result.status = "Two-ray ground reflection model";
    }

    return result;
}

RTXRFPropagation::PropagationResult RTXRFPropagation::computeOkumuraHata(const Transmitter &tx, const Receiver &rx) {
    PropagationResult result;

    real_T distance_km = (rx.position - tx.position).norm() / 1000.0;
    real_T freq_mhz = tx.frequency_hz / 1e6;
    real_T h_tx_m = tx.antenna_height_m;
    real_T h_rx_m = rx.antenna_height_m;

    // Okumura-Hata model for urban areas
    // Valid for: 150-1500 MHz, 1-20 km, Tx: 30-200m, Rx: 1-10m

    real_T a_h_rx = (1.1 * std::log10(freq_mhz) - 0.7) * h_rx_m - (1.56 * std::log10(freq_mhz) - 0.8);

    real_T L_urban = 69.55 + 26.16 * std::log10(freq_mhz) - 13.82 * std::log10(h_tx_m) - a_h_rx +
                     (44.9 - 6.55 * std::log10(h_tx_m)) * std::log10(distance_km);

    result.distance_m = distance_km * 1000.0;
    result.path_loss_db = L_urban;
    result.received_power_dbm = tx.power_dbm + tx.antenna_gain_dbi + rx.antenna_gain_dbi - L_urban;
    result.line_of_sight = false; // Empirical model assumes NLOS
    result.status = "Okumura-Hata urban model";

    return result;
}

real_T RTXRFPropagation::computeFreeSpacePathLoss(real_T distance_m, real_T frequency_hz) const {
    // Friis free space path loss formula: FSPL = 20*log10(d) + 20*log10(f) + 20*log10(4π/c)
    real_T wavelength = SPEED_OF_LIGHT / frequency_hz;
    real_T fspl_db =
        20.0 * std::log10(distance_m) + 20.0 * std::log10(frequency_hz) + 20.0 * std::log10(4.0 * PI / SPEED_OF_LIGHT);
    return fspl_db;
}

real_T RTXRFPropagation::computeAtmosphericLoss(real_T distance_m, real_T frequency_hz,
                                                WeatherCondition weather) const {
    real_T loss_db = 0.0;
    real_T freq_ghz = frequency_hz / 1e9;

    switch (weather) {
    case WeatherCondition::Clear:
        // Minimal loss for clear conditions
        loss_db = 0.001 * distance_m * freq_ghz; // ~0.1 dB/km at 1 GHz
        break;
    case WeatherCondition::LightRain:
        // Rain attenuation increases with frequency (especially > 10 GHz)
        loss_db = 0.01 * distance_m * std::pow(freq_ghz, 1.5);
        break;
    case WeatherCondition::ModerateRain:
        loss_db = 0.05 * distance_m * std::pow(freq_ghz, 1.5);
        break;
    case WeatherCondition::HeavyRain:
        loss_db = 0.2 * distance_m * std::pow(freq_ghz, 1.5);
        break;
    case WeatherCondition::Fog:
        // Fog scattering, more significant at mmWave
        loss_db = 0.03 * distance_m * std::pow(freq_ghz, 2.0);
        break;
    case WeatherCondition::Snow:
        // Less attenuation than rain
        loss_db = 0.03 * distance_m * std::pow(freq_ghz, 1.5);
        break;
    case WeatherCondition::Storm:
        // Combined severe effects
        loss_db = 0.5 * distance_m * std::pow(freq_ghz, 1.5);
        break;
    }

    return loss_db;
}

real_T RTXRFPropagation::computeMaterialPenetrationLoss(const MaterialProperties &material, real_T thickness_m,
                                                        real_T frequency_hz) const {
    // Simplified penetration loss model
    real_T loss_db = material.attenuation_db_per_m * thickness_m;

    // Frequency dependence (higher frequency = more loss)
    real_T freq_ghz = frequency_hz / 1e9;
    loss_db *= std::sqrt(freq_ghz);

    return loss_db;
}

real_T RTXRFPropagation::computeFresnelZone(real_T distance_m, real_T frequency_hz, uint32_t zone_number) const {
    real_T wavelength = SPEED_OF_LIGHT / frequency_hz;
    real_T fresnel_radius = std::sqrt(zone_number * wavelength * distance_m / 4.0);
    return fresnel_radius;
}

bool RTXRFPropagation::checkRTXAvailable() const {
    // TODO: Check for NVIDIA RTX GPU and OptiX support
    // For now, return false (would implement with CUDA/OptiX detection)
    return false;
}

void RTXRFPropagation::initializeRTXContext() {
    // TODO: Initialize OptiX context
    // This would include:
    // 1. Create OptiX context
    // 2. Load PTX/OptiX IR modules
    // 3. Create pipeline with ray generation, miss, and closest hit programs
    // 4. Build shader binding table
    // 5. Prepare acceleration structures

    Utils::log("RTX context initialization (placeholder)", Utils::kLogLevelInfo);
}

void RTXRFPropagation::buildRTXScene() {
    // TODO: Build OptiX acceleration structure from scene geometry
}

void RTXRFPropagation::registerJammer(const Transmitter &jammer) {
    std::lock_guard<std::mutex> lock(mutex_);
    jammers_.push_back(jammer);
}

real_T RTXRFPropagation::computeInterference(const Receiver &rx, real_T signal_frequency_hz) {
    std::lock_guard<std::mutex> lock(mutex_);

    real_T total_interference_dbm = -1000.0; // Very low

    for (const auto &jammer : jammers_) {
        if (jammer.active) {
            PropagationResult result = computePropagation(jammer, rx);
            // Sum interference power (linear domain)
            real_T interference_mw = std::pow(10.0, result.received_power_dbm / 10.0);
            real_T total_mw = std::pow(10.0, total_interference_dbm / 10.0);
            total_interference_dbm = 10.0 * std::log10(total_mw + interference_mw);
        }
    }

    return total_interference_dbm;
}

real_T RTXRFPropagation::computeSINR(const std::string &tx_id, const std::string &rx_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto tx_it = transmitters_.find(tx_id);
    auto rx_it = receivers_.find(rx_id);

    if (tx_it == transmitters_.end() || rx_it == receivers_.end()) {
        return -1000.0; // Invalid
    }

    PropagationResult signal = computePropagation(tx_it->second, rx_it->second);
    real_T interference = computeInterference(rx_it->second, tx_it->second.frequency_hz);
    real_T noise_dbm = -174.0 + 10.0 * std::log10(tx_it->second.frequency_hz) + rx_it->second.noise_figure_db;

    // Convert to linear, sum interference and noise, convert back
    real_T signal_mw = std::pow(10.0, signal.received_power_dbm / 10.0);
    real_T interference_mw = std::pow(10.0, interference / 10.0);
    real_T noise_mw = std::pow(10.0, noise_dbm / 10.0);

    real_T sinr = 10.0 * std::log10(signal_mw / (interference_mw + noise_mw));
    return sinr;
}

void RTXRFPropagation::setTransmitterVelocity(const std::string &tx_id, const Vector3r &velocity_m_s) {
    std::lock_guard<std::mutex> lock(mutex_);
    transmitter_velocities_[tx_id] = velocity_m_s;
}

void RTXRFPropagation::setReceiverVelocity(const std::string &rx_id, const Vector3r &velocity_m_s) {
    std::lock_guard<std::mutex> lock(mutex_);
    receiver_velocities_[rx_id] = velocity_m_s;
}

real_T RTXRFPropagation::computeDopplerShift(const Transmitter &tx, const Receiver &rx, const Vector3r &tx_velocity,
                                             const Vector3r &rx_velocity) {
    // Doppler shift: f_d = f_c * (v_r - v_t) / c
    Vector3r direction = (rx.position - tx.position).normalized();
    real_T relative_velocity = direction.dot(rx_velocity - tx_velocity);
    real_T doppler_hz = tx.frequency_hz * relative_velocity / SPEED_OF_LIGHT;
    return doppler_hz;
}

void RTXRFPropagation::setScene(void *unreal_scene_ptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    scene_ptr_ = unreal_scene_ptr;
    if (optix_context_ != nullptr) {
        buildRTXScene();
    }
}

void RTXRFPropagation::updateSceneGeometry() {
    if (optix_context_ != nullptr) {
        buildRTXScene();
    }
}

void RTXRFPropagation::setTerrainElevation(const std::vector<std::vector<real_T>> &elevation_grid_m) {
    std::lock_guard<std::mutex> lock(mutex_);
    terrain_elevation_ = elevation_grid_m;
}

std::vector<Vector3r> RTXRFPropagation::getRayPath(const std::string &tx_id, const std::string &rx_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto key = std::make_pair(tx_id, rx_id);
    auto it = cached_ray_paths_.find(key);

    if (it != cached_ray_paths_.end()) {
        return it->second;
    }

    return std::vector<Vector3r>();
}

real_T RTXRFPropagation::getAverageComputeTimeMs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (num_queries_ == 0)
        return 0.0;
    return total_compute_time_ms_ / static_cast<real_T>(num_queries_);
}

uint32_t RTXRFPropagation::getRTXCoreUtilization() const {
    // TODO: Query GPU utilization via NVML
    return 0; // Placeholder
}

void RTXRFPropagation::resetStatistics() {
    std::lock_guard<std::mutex> lock(mutex_);
    total_compute_time_ms_ = 0.0;
    num_queries_ = 0;
}

std::vector<Vector3r> RTXRFPropagation::computeCoverageArea(const std::string &tx_id, real_T min_rssi_dbm,
                                                            real_T grid_resolution_m) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<Vector3r> coverage_points;

    auto tx_it = transmitters_.find(tx_id);
    if (tx_it == transmitters_.end()) {
        return coverage_points;
    }

    // TODO: Grid search around transmitter, compute RSSI at each point
    // Return points where RSSI > min_rssi_dbm

    return coverage_points;
}

real_T RTXRFPropagation::computeOutageProbability(const std::string &tx_id, const std::string &rx_id,
                                                  real_T required_snr_db) {
    // Simplified outage probability calculation
    PropagationResult result = computePropagation(tx_id, rx_id);

    if (result.signal_to_noise_db >= required_snr_db) {
        return 0.0; // No outage
    } else {
        // Simple model: probability increases with SNR deficit
        real_T deficit_db = required_snr_db - result.signal_to_noise_db;
        real_T outage_prob = std::min(1.0, deficit_db / 20.0);
        return outage_prob;
    }
}

} // namespace autonomysim
