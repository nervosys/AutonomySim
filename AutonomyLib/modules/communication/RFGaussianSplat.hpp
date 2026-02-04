#ifndef autonomylib_modules_communication_RFGaussianSplat_hpp
#define autonomylib_modules_communication_RFGaussianSplat_hpp

#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include <memory>
#include <mutex>
#include <vector>

namespace autonomysim {

/**
 * @brief 3D RF Gaussian Splatting for real-time RF field visualization
 *
 * Integrates NVIDIA instant-rm neural radiance maps with 3D Gaussian splatting
 * for efficient visualization and interpolation of radio frequency propagation.
 *
 * Based on:
 * - instant-rm: Fast differentiable radio maps (NVLabs)
 * - 3D Gaussian Splatting: Real-time radiance field rendering
 *
 * Features:
 * - Neural representation of RF fields using radiance maps
 * - Gaussian splat rendering for real-time visualization
 * - Differentiable RF field gradients for optimization
 * - Efficient GPU-accelerated rendering
 * - Multi-frequency band support
 * - Coverage prediction and path loss mapping
 */
class RFGaussianSplat {
  public:
    /**
     * @brief 3D Gaussian splat representing a localized RF field
     */
    struct Gaussian3D {
        Vector3r position;          // Center position [m]
        Vector3r scale;             // Scale along principal axes [m]
        Quaternionr rotation;       // Orientation quaternion
        real_T signal_strength_dbm; // RF signal strength [dBm]
        real_T frequency_hz;        // Carrier frequency [Hz]
        real_T opacity;             // Splat opacity [0-1]
        Vector3r color_rgb;         // RGB color for visualization

        // Covariance matrix (computed from scale and rotation)
        // Σ = R * S * S^T * R^T
        real_T covariance[6]; // Upper triangular: [σ_xx, σ_xy, σ_xz, σ_yy, σ_yz, σ_zz]
    };

    /**
     * @brief Configuration for RF Gaussian splatting
     */
    struct RFSplatConfig {
        uint32_t num_gaussians;        // Number of Gaussian splats
        real_T learning_rate;          // For gradient descent optimization
        uint32_t max_iterations;       // Training iterations
        bool enable_sh_coefficients;   // Spherical harmonics for view-dependent effects
        uint32_t sh_degree;            // Spherical harmonics degree (0-3)
        real_T position_lr_scale;      // Learning rate scale for positions
        real_T opacity_lr_scale;       // Learning rate scale for opacity
        real_T scale_lr_scale;         // Learning rate scale for scales
        real_T rotation_lr_scale;      // Learning rate scale for rotations
        bool adaptive_density_control; // Enable/disable adaptive Gaussian splitting
        real_T densify_grad_threshold; // Gradient threshold for densification
        uint32_t densify_interval;     // Iterations between densification
        real_T opacity_reset_interval; // Reset opacity every N iterations
    };

    /**
     * @brief Neural radiance map from instant-rm
     */
    struct NeuralRadianceMap {
        std::vector<real_T> path_loss_map;              // [num_cells_x, num_cells_y]
        std::vector<real_T> rms_delay_spread;           // [num_cells_x, num_cells_y]
        std::vector<Vector3r> mean_direction_arrival;   // [num_cells_x, num_cells_y]
        std::vector<Vector3r> mean_direction_departure; // [num_cells_x, num_cells_y]

        Vector3r measurement_plane_center;
        Vector3r measurement_plane_orientation;
        Vector2r measurement_plane_size;
        Vector2r cell_size;
        uint32_t num_cells_x;
        uint32_t num_cells_y;

        real_T frequency_hz;
        real_T tx_power_dbm;
        Vector3r tx_position;
        Vector3r tx_orientation;
    };

    /**
     * @brief Ray tube for instant-rm ray tracing
     */
    struct RayTube {
        Vector3r origin;
        Vector3r direction;
        real_T field_strength; // Stokes vector magnitude
        real_T rho_1;          // First radius of curvature [m]
        real_T rho_2;          // Second radius of curvature [m]
        real_T solid_angle;    // [steradians]
        real_T path_length;    // [m]
        bool active;
    };

    /**
     * @brief Training data point for fitting Gaussians
     */
    struct TrainingPoint {
        Vector3r position;
        real_T signal_strength_dbm;
        real_T weight; // Sample importance weight
    };

    RFGaussianSplat();
    ~RFGaussianSplat();

    // Initialization and configuration
    bool initialize(const RFSplatConfig &config);
    void shutdown();
    void setConfig(const RFSplatConfig &config);
    RFSplatConfig getConfig() const;

    // Neural radiance map integration (instant-rm)
    void setRadianceMap(const NeuralRadianceMap &radiance_map);
    NeuralRadianceMap getRadianceMap() const;
    bool trainFromRadianceMap(const NeuralRadianceMap &radiance_map);

    // Gaussian splat management
    void addGaussian(const Gaussian3D &gaussian);
    void removeGaussian(uint32_t index);
    void updateGaussian(uint32_t index, const Gaussian3D &gaussian);
    Gaussian3D getGaussian(uint32_t index) const;
    uint32_t getNumGaussians() const;
    std::vector<Gaussian3D> getAllGaussians() const;

    // Training from measurements
    bool trainFromMeasurements(const std::vector<TrainingPoint> &training_data);
    void resetTraining();
    real_T computeLoss() const;

    // RF field queries
    real_T querySignalStrength(const Vector3r &position) const;
    Vector3r queryGradient(const Vector3r &position) const;
    std::vector<real_T> queryCoverageArea(real_T min_signal_dbm, real_T resolution_m) const;

    // Rendering and visualization
    void renderToImage(void *image_buffer, uint32_t width, uint32_t height, const Vector3r &camera_position,
                       const Quaternionr &camera_rotation) const;
    void renderToPointCloud(std::vector<Vector3r> &positions, std::vector<real_T> &signal_strengths) const;

    // Optimization (gradient descent on Gaussians)
    void optimizeStep();
    void optimizeMultiStep(uint32_t num_steps);

    // Adaptive density control
    void densifyGaussians(); // Split high-gradient Gaussians
    void pruneGaussians();   // Remove low-opacity Gaussians
    void resetOpacity();     // Periodic opacity reset

    // Instant-rm ray tracing utilities
    std::vector<RayTube> traceRays(const Vector3r &tx_position, const Vector3r &tx_orientation, uint32_t num_samples,
                                   uint32_t max_depth) const;

    // Export/Import
    bool exportToFile(const std::string &filename) const;
    bool importFromFile(const std::string &filename);

    // Statistics
    real_T getAverageSignalStrength() const;
    real_T getMinSignalStrength() const;
    real_T getMaxSignalStrength() const;
    uint32_t getTrainingIteration() const;
    real_T getCurrentLoss() const;

  private:
    // Internal state
    bool initialized_;
    RFSplatConfig config_;
    std::vector<Gaussian3D> gaussians_;
    NeuralRadianceMap radiance_map_;
    uint32_t training_iteration_;
    real_T current_loss_;

    // Gradients for optimization
    std::vector<Vector3r> position_gradients_;
    std::vector<Vector3r> scale_gradients_;
    std::vector<Quaternionr> rotation_gradients_;
    std::vector<real_T> opacity_gradients_;
    std::vector<real_T> signal_gradients_;

    // Thread safety
    mutable std::mutex mutex_;

    // Private helper functions
    void initializeGaussiansFromRadianceMap(const NeuralRadianceMap &radiance_map);
    void computeCovarianceMatrix(Gaussian3D &gaussian);
    real_T evaluateGaussian(const Gaussian3D &gaussian, const Vector3r &position) const;
    void computeGaussianGradients(uint32_t gaussian_idx, const std::vector<TrainingPoint> &training_data);
    void updateGaussianParameters(uint32_t gaussian_idx, real_T learning_rate);

    // Color mapping for visualization (signal strength to RGB)
    Vector3r signalStrengthToRGB(real_T signal_dbm, real_T min_dbm, real_T max_dbm) const;

    // Spherical harmonics (for view-dependent effects)
    void initializeSphericalHarmonics();
    real_T evaluateSphericalHarmonic(uint32_t gaussian_idx, const Vector3r &view_direction) const;

    // Radiance map sampling
    real_T sampleRadianceMap(const Vector3r &position) const;
    Vector3r sampleDirectionArrival(const Vector3r &position) const;
};

} // namespace autonomysim

#endif // autonomylib_modules_communication_RFGaussianSplat_hpp
