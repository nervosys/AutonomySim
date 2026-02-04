// InstantRMBridge.hpp - Python bridge for NVIDIA instant-rm integration
// Provides C++ interface to instant-rm Python library for neural radiance map generation

#ifndef autonomylib_modules_communication_InstantRMBridge_hpp
#define autonomylib_modules_communication_InstantRMBridge_hpp

#include "RFGaussianSplat.hpp"
#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include <memory>
#include <string>
#include <vector>

// Forward declare Python types to avoid requiring Python.h in header
struct _object;
typedef _object PyObject;

namespace autonomysim {

/**
 * @brief Bridge to NVIDIA instant-rm Python library
 *
 * Provides C++ interface to instant-rm for generating neural radiance maps
 * of RF propagation using Mitsuba 3 and Dr.Jit.
 *
 * Requires:
 * - Python 3.8+
 * - Mitsuba 3 (cuda_ad_mono_polarized variant)
 * - instant-rm package
 *
 * Usage:
 *   InstantRMBridge bridge;
 *   bridge.initialize();
 *   auto radiance_map = bridge.computeRadianceMap(tx_pos, tx_power, frequency, scene);
 */
class InstantRMBridge {
  public:
    /**
     * @brief Scene configuration for instant-rm
     */
    struct SceneConfig {
        std::string scene_file;                    // Mitsuba scene XML file
        std::vector<Vector3r> obstacle_positions;  // Obstacle centers
        std::vector<Vector3r> obstacle_sizes;      // Obstacle dimensions
        std::vector<real_T> obstacle_permittivity; // Relative permittivity (εr)
        std::vector<real_T> obstacle_conductivity; // Conductivity (σ) [S/m]
        std::string material_type;                 // "smooth", "lambertian", "backscattering"

        SceneConfig() : material_type("lambertian") {}
    };

    /**
     * @brief Antenna pattern configuration
     */
    struct AntennaConfig {
        std::string pattern_type; // "isotropic", "dipole", "hw_dipole", "tr38901"
        real_T gain_dbi;          // Antenna gain [dBi]
        Vector3r orientation;     // Antenna orientation (direction vector)
        real_T beamwidth_deg;     // 3dB beamwidth [degrees] (for directional)

        AntennaConfig() : pattern_type("isotropic"), gain_dbi(0.0), orientation(0, 0, 1), beamwidth_deg(360.0) {}
    };

    /**
     * @brief Measurement plane configuration
     */
    struct MeasurementPlane {
        Vector3r center;       // Plane center [m]
        Vector3r normal;       // Plane normal (direction)
        Vector2r size;         // Plane size [m] (width, height)
        uint32_t resolution_x; // Number of cells in X
        uint32_t resolution_y; // Number of cells in Y
        real_T height_offset;  // Offset from ground [m]

        MeasurementPlane()
            : center(0, 0, 1.5), normal(0, 0, 1), size(100, 100), resolution_x(128), resolution_y(128),
              height_offset(1.5) {}
    };

    /**
     * @brief Ray tracing configuration
     */
    struct TracingConfig {
        uint32_t num_samples;      // Number of ray samples (Fibonacci lattice)
        uint32_t max_depth;        // Maximum ray bounce depth
        bool use_rbp;              // Use radiative backpropagation for gradients
        bool compute_delay_spread; // Compute RMS delay spread
        bool compute_directions;   // Compute mean DoA/DoD
        real_T russian_roulette;   // Russian roulette threshold [0-1]

        TracingConfig()
            : num_samples(1024), max_depth(5), use_rbp(false), compute_delay_spread(true), compute_directions(true),
              russian_roulette(0.95) {}
    };

    // Constructor/Destructor
    InstantRMBridge();
    ~InstantRMBridge();

    // Initialization
    bool initialize();
    void shutdown();
    bool isInitialized() const { return initialized_; }

    // Configuration
    void setSceneConfig(const SceneConfig &config);
    void setAntennaConfig(const AntennaConfig &config);
    void setMeasurementPlane(const MeasurementPlane &plane);
    void setTracingConfig(const TracingConfig &config);

    // Radiance map generation
    RFGaussianSplat::NeuralRadianceMap computeRadianceMap(const Vector3r &tx_position, real_T tx_power_dbm,
                                                          real_T frequency_hz);

    RFGaussianSplat::NeuralRadianceMap computeRadianceMap(const Vector3r &tx_position, real_T tx_power_dbm,
                                                          real_T frequency_hz, const SceneConfig &scene);

    // Multi-transmitter scenarios
    RFGaussianSplat::NeuralRadianceMap computeMultiTxRadianceMap(const std::vector<Vector3r> &tx_positions,
                                                                 const std::vector<real_T> &tx_powers_dbm,
                                                                 real_T frequency_hz);

    // Gradient computation (via RBP)
    bool computeGradients(const RFGaussianSplat::NeuralRadianceMap &radiance_map,
                          std::vector<Vector3r> &position_gradients);

    // Utility functions
    bool loadScene(const std::string &scene_file);
    bool saveRadianceMap(const RFGaussianSplat::NeuralRadianceMap &map, const std::string &filename);
    bool loadRadianceMap(RFGaussianSplat::NeuralRadianceMap &map, const std::string &filename);

    // Statistics
    real_T getLastComputeTime() const { return last_compute_time_ms_; }
    uint32_t getRayCount() const { return ray_count_; }

  private:
    // Python integration
    bool initializePython();
    void shutdownPython();
    PyObject *convertVector3ToPython(const Vector3r &vec);
    Vector3r convertPythonToVector3(PyObject *obj);
    PyObject *convertSceneConfigToPython(const SceneConfig &config);

    // Helper functions
    RFGaussianSplat::NeuralRadianceMap extractRadianceMapFromPython(PyObject *result);
    bool callInstantRM(const Vector3r &tx_position, real_T tx_power_dbm, real_T frequency_hz, PyObject *&result);

    // Python objects
    PyObject *instant_rm_module_;
    PyObject *map_tracer_class_;
    PyObject *map_tracer_instance_;
    PyObject *scene_object_;

    // Configuration
    SceneConfig scene_config_;
    AntennaConfig antenna_config_;
    MeasurementPlane measurement_plane_;
    TracingConfig tracing_config_;

    // State
    bool initialized_;
    bool python_initialized_;
    real_T last_compute_time_ms_;
    uint32_t ray_count_;

    // Thread safety
    mutable std::mutex mutex_;
};

} // namespace autonomysim

#endif // autonomylib_modules_communication_InstantRMBridge_hpp
