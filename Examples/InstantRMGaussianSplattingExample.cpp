// Example: Integrate instant-rm neural radiance maps with 3D Gaussian splatting
// for real-time RF field visualization and optimization

#include "ai/AgenticSwarmController.hpp"
#include "common/Common.hpp"
#include "communication/RFGaussianSplat.hpp"
#include "communication/RTXRFPropagation.hpp"
#include <fstream>
#include <iostream>

using namespace autonomysim;

// Placeholder for instant-rm Python bridge
// In production, this would use Python C API or pybind11
class InstantRMBridge {
  public:
    struct SceneConfig {
        std::string scene_file;
        std::vector<Vector3r> obstacles;
        std::vector<real_T> obstacle_materials; // Permittivity
    };

    bool initialize() {
        std::cout << "[InstantRM] Initializing Python bridge..." << std::endl;
        // TODO: Initialize Python interpreter and import instant_rm
        // Py_Initialize();
        // instant_rm_module_ = PyImport_ImportModule("instant_rm");
        return true;
    }

    RFGaussianSplat::NeuralRadianceMap computeRadianceMap(const Vector3r &tx_position, real_T tx_power_dbm,
                                                          real_T frequency_hz, const SceneConfig &scene) {
        std::cout << "[InstantRM] Computing neural radiance map..." << std::endl;
        std::cout << "  TX Position: " << tx_position.transpose() << std::endl;
        std::cout << "  TX Power: " << tx_power_dbm << " dBm" << std::endl;
        std::cout << "  Frequency: " << frequency_hz / 1e9 << " GHz" << std::endl;

        // TODO: Call instant-rm Python API
        // For now, generate placeholder data
        RFGaussianSplat::NeuralRadianceMap radiance_map;

        // Measurement plane: 200m x 200m at 1.5m height
        radiance_map.measurement_plane_center = Vector3r(0, 0, 1.5);
        radiance_map.measurement_plane_orientation = Vector3r(0, 0, 1);
        radiance_map.measurement_plane_size = Vector2r(200, 200);
        radiance_map.num_cells_x = 128;
        radiance_map.num_cells_y = 128;
        radiance_map.frequency_hz = frequency_hz;
        radiance_map.tx_power_dbm = tx_power_dbm;

        // Generate placeholder path loss map (Friis + distance)
        uint32_t num_cells = radiance_map.num_cells_x * radiance_map.num_cells_y;
        radiance_map.path_loss_map.resize(num_cells);
        radiance_map.rms_delay_spread.resize(num_cells);
        radiance_map.mean_direction_arrival.resize(num_cells);
        radiance_map.mean_direction_departure.resize(num_cells);

        real_T wavelength = 3e8 / frequency_hz;

        for (uint32_t y = 0; y < radiance_map.num_cells_y; ++y) {
            for (uint32_t x = 0; x < radiance_map.num_cells_x; ++x) {
                uint32_t idx = y * radiance_map.num_cells_x + x;

                // Cell position in world coordinates
                real_T u = (x + 0.5) / radiance_map.num_cells_x - 0.5;
                real_T v = (y + 0.5) / radiance_map.num_cells_y - 0.5;
                Vector3r cell_pos =
                    radiance_map.measurement_plane_center + Vector3r(u * radiance_map.measurement_plane_size.x(),
                                                                     v * radiance_map.measurement_plane_size.y(), 0);

                // Distance from transmitter
                real_T distance = (cell_pos - tx_position).norm();

                // Friis path loss: PL = 20*log10(d) + 20*log10(f) - 147.55
                real_T path_loss = 20.0 * std::log10(distance + 1e-3) + 20.0 * std::log10(frequency_hz) - 147.55;

                radiance_map.path_loss_map[idx] = path_loss;

                // Placeholder RMS delay spread (increases with distance)
                radiance_map.rms_delay_spread[idx] = distance * 10e-9; // 10 ns/m

                // Direction from TX to cell
                Vector3r direction = (cell_pos - tx_position).normalized();
                radiance_map.mean_direction_arrival[idx] = direction;
                radiance_map.mean_direction_departure[idx] = -direction;
            }
        }

        std::cout << "[InstantRM] Radiance map computed: " << radiance_map.num_cells_x << "x"
                  << radiance_map.num_cells_y << " cells" << std::endl;

        return radiance_map;
    }

  private:
    // PyObject* instant_rm_module_;
};

// Example 1: Train Gaussian representation from radiance map
void example_train_gaussians() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Example 1: Train Gaussians from Radiance Map" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Initialize instant-rm bridge
    InstantRMBridge instant_rm;
    instant_rm.initialize();

    // Configure scene
    InstantRMBridge::SceneConfig scene;
    scene.scene_file = "urban_environment.xml";
    // Add some obstacles
    scene.obstacles.push_back(Vector3r(20, 0, 0));
    scene.obstacles.push_back(Vector3r(-15, 30, 0));
    scene.obstacle_materials.push_back(5.0); // Concrete (εr ≈ 5)
    scene.obstacle_materials.push_back(5.0);

    // Transmitter configuration
    Vector3r tx_position(0, 0, 10); // 10m height
    real_T tx_power_dbm = 30.0;     // 1W
    real_T frequency_hz = 2.4e9;    // 2.4 GHz

    // Generate neural radiance map using instant-rm
    auto radiance_map = instant_rm.computeRadianceMap(tx_position, tx_power_dbm, frequency_hz, scene);

    // Initialize Gaussian splat system
    RFGaussianSplat rf_splat;
    RFGaussianSplat::RFSplatConfig config;
    config.num_gaussians = 10000;
    config.learning_rate = 0.01;
    config.adaptive_density_control = true;
    config.densify_grad_threshold = 0.0002;
    config.densify_interval = 100;
    config.max_iterations = 1000;
    config.position_lr_scale = 1.0;
    config.scale_lr_scale = 0.5;
    config.opacity_lr_scale = 0.1;

    rf_splat.initialize(config);

    // Train Gaussian representation
    std::cout << "\nTraining Gaussian representation..." << std::endl;
    rf_splat.trainFromRadianceMap(radiance_map);

    // Query signal strength at various locations
    std::cout << "\nQuerying signal strength:" << std::endl;
    std::vector<Vector3r> query_points = {Vector3r(10, 0, 1.5), Vector3r(0, 20, 1.5), Vector3r(-15, -15, 1.5),
                                          Vector3r(50, 50, 1.5)};

    for (const auto &point : query_points) {
        real_T signal = rf_splat.querySignalStrength(point);
        Vector3r gradient = rf_splat.queryGradient(point);

        std::cout << "  Position: " << point.transpose() << " -> Signal: " << signal << " dBm"
                  << ", Gradient magnitude: " << gradient.norm() << " dB/m" << std::endl;
    }

    // Export trained model
    rf_splat.exportToFile("rf_field_gaussians.bin");
    std::cout << "\nGaussian model exported to rf_field_gaussians.bin" << std::endl;

    // Statistics
    std::cout << "\nStatistics:" << std::endl;
    std::cout << "  Total Gaussians: " << rf_splat.getNumGaussians() << std::endl;
    std::cout << "  Average signal: " << rf_splat.getAverageSignalStrength() << " dBm" << std::endl;
    std::cout << "  Min signal: " << rf_splat.getMinSignalStrength() << " dBm" << std::endl;
    std::cout << "  Max signal: " << rf_splat.getMaxSignalStrength() << " dBm" << std::endl;
    std::cout << "  Training iterations: " << rf_splat.getTrainingIteration() << std::endl;
    std::cout << "  Final loss: " << rf_splat.getCurrentLoss() << " dB^2" << std::endl;
}

// Example 2: Optimize transmitter placement for coverage
void example_optimize_coverage() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Example 2: Optimize Transmitter Coverage" << std::endl;
    std::cout << "========================================\n" << std::endl;

    InstantRMBridge instant_rm;
    instant_rm.initialize();

    RFGaussianSplat rf_splat;
    RFGaussianSplat::RFSplatConfig config;
    config.num_gaussians = 5000;
    config.learning_rate = 0.02;
    config.max_iterations = 500;
    rf_splat.initialize(config);

    // Target coverage area
    Vector3r coverage_center(25, 25, 0);
    real_T coverage_radius = 30.0; // meters
    real_T min_signal_dbm = -80.0;

    // Optimization: Gradient ascent on coverage area
    Vector3r tx_position(0, 0, 10);
    real_T step_size = 1.0; // meters
    uint32_t max_iterations = 20;

    std::cout << "Optimizing transmitter position for coverage..." << std::endl;
    std::cout << "Target area: center=" << coverage_center.transpose() << ", radius=" << coverage_radius << "m"
              << std::endl;
    std::cout << "Min signal: " << min_signal_dbm << " dBm\n" << std::endl;

    for (uint32_t iter = 0; iter < max_iterations; ++iter) {
        // Generate radiance map for current TX position
        InstantRMBridge::SceneConfig scene;
        auto radiance_map = instant_rm.computeRadianceMap(tx_position, 30.0, 2.4e9, scene);

        // Train Gaussians
        rf_splat.trainFromRadianceMap(radiance_map);

        // Evaluate coverage
        std::vector<real_T> coverage = rf_splat.queryCoverageArea(min_signal_dbm, 2.0);
        real_T coverage_score = coverage.size();

        std::cout << "Iteration " << iter << ": TX Position=" << tx_position.transpose()
                  << ", Coverage=" << coverage_score << " points" << std::endl;

        // Compute gradient of coverage w.r.t. TX position (numerical)
        Vector3r grad = Vector3r::Zero();
        real_T epsilon = 0.5; // meters

        for (int dim = 0; dim < 3; ++dim) {
            Vector3r perturbed = tx_position;
            perturbed[dim] += epsilon;

            auto perturbed_map = instant_rm.computeRadianceMap(perturbed, 30.0, 2.4e9, scene);
            rf_splat.trainFromRadianceMap(perturbed_map);

            std::vector<real_T> perturbed_coverage = rf_splat.queryCoverageArea(min_signal_dbm, 2.0);
            real_T perturbed_score = perturbed_coverage.size();

            grad[dim] = (perturbed_score - coverage_score) / epsilon;
        }

        // Move TX in direction of better coverage
        if (grad.norm() > 1e-3) {
            tx_position += grad.normalized() * step_size;
        } else {
            std::cout << "Converged!" << std::endl;
            break;
        }
    }

    std::cout << "\nOptimal TX Position: " << tx_position.transpose() << std::endl;
}

// Example 3: Real-time RF visualization with Gaussian splatting
void example_realtime_visualization() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Example 3: Real-Time RF Visualization" << std::endl;
    std::cout << "========================================\n" << std::endl;

    RFGaussianSplat rf_splat;
    RFGaussianSplat::RFSplatConfig config;
    config.num_gaussians = 20000;
    config.learning_rate = 0.01;
    config.max_iterations = 2000;
    rf_splat.initialize(config);

    // Load pre-trained Gaussian model
    if (rf_splat.importFromFile("rf_field_gaussians.bin")) {
        std::cout << "Loaded pre-trained Gaussian model" << std::endl;
    } else {
        std::cout << "Training new model..." << std::endl;
        InstantRMBridge instant_rm;
        instant_rm.initialize();

        InstantRMBridge::SceneConfig scene;
        auto radiance_map = instant_rm.computeRadianceMap(Vector3r(0, 0, 10), 30.0, 2.4e9, scene);

        rf_splat.trainFromRadianceMap(radiance_map);
    }

    // Render to image from different viewpoints
    uint32_t width = 1920;
    uint32_t height = 1080;
    std::vector<float> framebuffer(width * height * 4);

    std::vector<Vector3r> camera_positions = {Vector3r(100, 100, 50), Vector3r(0, 100, 30), Vector3r(-80, 80, 40)};

    for (size_t i = 0; i < camera_positions.size(); ++i) {
        std::cout << "\nRendering view " << (i + 1) << " from " << camera_positions[i].transpose() << std::endl;

        // TODO: Full implementation would render to framebuffer
        // rf_splat.renderToImage(framebuffer.data(), width, height,
        //                        camera_positions[i], Quaternionr::Identity());

        // Export to point cloud instead (for visualization in other tools)
        std::vector<Vector3r> positions;
        std::vector<real_T> signal_strengths;
        rf_splat.renderToPointCloud(positions, signal_strengths);

        std::cout << "  Exported " << positions.size() << " points" << std::endl;

        // Save to PLY format
        std::string filename = "rf_field_view_" + std::to_string(i + 1) + ".ply";
        std::ofstream ply_file(filename);
        ply_file << "ply\n";
        ply_file << "format ascii 1.0\n";
        ply_file << "element vertex " << positions.size() << "\n";
        ply_file << "property float x\n";
        ply_file << "property float y\n";
        ply_file << "property float z\n";
        ply_file << "property float signal_dbm\n";
        ply_file << "end_header\n";

        for (size_t j = 0; j < positions.size(); ++j) {
            ply_file << positions[j].x() << " " << positions[j].y() << " " << positions[j].z() << " "
                     << signal_strengths[j] << "\n";
        }
        ply_file.close();

        std::cout << "  Saved to " << filename << std::endl;
    }
}

// Example 4: Swarm communication optimization
void example_swarm_communication() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Example 4: Autonomous Swarm Communication" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Initialize swarm controller
    AgenticSwarmController swarm;
    AgenticSwarmController::SwarmConfig swarm_config;
    swarm_config.num_vehicles = 5;
    swarm_config.formation_type = AgenticSwarmController::FormationType::Grid;
    swarm.initialize(swarm_config);

    // Initialize RF Gaussian splat
    RFGaussianSplat rf_splat;
    RFGaussianSplat::RFSplatConfig splat_config;
    splat_config.num_gaussians = 8000;
    splat_config.learning_rate = 0.015;
    splat_config.max_iterations = 800;
    rf_splat.initialize(splat_config);

    // instant-rm bridge
    InstantRMBridge instant_rm;
    instant_rm.initialize();

    std::cout << "Optimizing swarm formation for communication..." << std::endl;

    // Optimization loop
    for (uint32_t iter = 0; iter < 30; ++iter) {
        // Get current vehicle positions
        std::vector<Vector3r> vehicle_positions;
        for (uint32_t i = 0; i < swarm_config.num_vehicles; ++i) {
            // TODO: Get actual vehicle positions from swarm
            vehicle_positions.push_back(Vector3r(i * 10.0, i * 10.0, 10.0));
        }

        // Generate radiance map with each vehicle as transmitter
        real_T total_connectivity = 0.0;

        for (uint32_t tx_idx = 0; tx_idx < vehicle_positions.size(); ++tx_idx) {
            InstantRMBridge::SceneConfig scene;
            auto radiance_map = instant_rm.computeRadianceMap(vehicle_positions[tx_idx], 20.0, 5.8e9, scene); // 5.8 GHz

            rf_splat.trainFromRadianceMap(radiance_map);

            // Evaluate connectivity to other vehicles
            for (uint32_t rx_idx = 0; rx_idx < vehicle_positions.size(); ++rx_idx) {
                if (rx_idx == tx_idx)
                    continue;

                real_T signal = rf_splat.querySignalStrength(vehicle_positions[rx_idx]);
                if (signal > -90.0) { // Minimum viable signal
                    total_connectivity += 1.0;
                }
            }
        }

        real_T connectivity_score = total_connectivity / (vehicle_positions.size() * (vehicle_positions.size() - 1));

        std::cout << "Iteration " << iter << ": Connectivity=" << connectivity_score * 100 << "%" << std::endl;

        // TODO: Adjust swarm formation based on RF gradient
        // swarm.updateFormation(...);
    }

    std::cout << "\nSwarm formation optimized for communication" << std::endl;
}

// Example 5: Compare analytical vs. neural RF propagation
void example_comparison() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Example 5: Analytical vs. Neural Comparison" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Analytical: RTXRFPropagation
    RTXRFPropagation rtx_rf;
    RTXRFPropagation::PropagationConfig rtx_config;
    rtx_config.frequency_hz = 2.4e9;
    rtx_config.tx_power_dbm = 30.0;
    rtx_config.model = RTXRFPropagation::PropagationModel::TwoRay;
    rtx_rf.initialize(rtx_config);

    // Neural: RFGaussianSplat + instant-rm
    RFGaussianSplat rf_splat;
    RFGaussianSplat::RFSplatConfig splat_config;
    splat_config.num_gaussians = 5000;
    splat_config.max_iterations = 500;
    rf_splat.initialize(splat_config);

    InstantRMBridge instant_rm;
    instant_rm.initialize();

    Vector3r tx_position(0, 0, 10);
    InstantRMBridge::SceneConfig scene;
    auto radiance_map = instant_rm.computeRadianceMap(tx_position, 30.0, 2.4e9, scene);
    rf_splat.trainFromRadianceMap(radiance_map);

    // Compare predictions
    std::cout << "\nComparing predictions:\n" << std::endl;
    std::cout << "Position (m)           | Analytical (dBm) | Neural (dBm) | Error (dB)" << std::endl;
    std::cout << "----------------------|------------------|--------------|------------" << std::endl;

    std::vector<Vector3r> test_points = {Vector3r(10, 0, 1.5), Vector3r(20, 0, 1.5), Vector3r(50, 0, 1.5),
                                         Vector3r(0, 30, 1.5), Vector3r(25, 25, 1.5)};

    for (const auto &point : test_points) {
        real_T distance = (point - tx_position).norm();
        real_T analytical_signal = rtx_rf.computeReceivedPower(distance, 0.0);
        real_T neural_signal = rf_splat.querySignalStrength(point);
        real_T error = std::abs(analytical_signal - neural_signal);

        printf("(%.1f, %.1f, %.1f)  | %16.2f | %12.2f | %10.2f\n", point.x(), point.y(), point.z(), analytical_signal,
               neural_signal, error);
    }

    std::cout << "\nMethod comparison:" << std::endl;
    std::cout << "  Analytical (RTX): Fast, physics-based, good extrapolation" << std::endl;
    std::cout << "  Neural (Gaussian): Learned from data, excellent interpolation" << std::endl;
}

int main(int argc, char *argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "instant-rm + Gaussian Splatting Examples" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        // Run examples
        example_train_gaussians();
        example_optimize_coverage();
        example_realtime_visualization();
        example_swarm_communication();
        example_comparison();

        std::cout << "\n========================================" << std::endl;
        std::cout << "All examples completed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
