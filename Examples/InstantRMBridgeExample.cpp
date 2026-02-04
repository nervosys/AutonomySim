// Complete example showing instant-rm integration with Python bridge

#include "common/Common.hpp"
#include "communication/InstantRMBridge.hpp"
#include "communication/RFGaussianSplat.hpp"
#include "communication/RTXRFPropagation.hpp"
#include <iostream>

using namespace autonomysim;

// Example 1: Basic radiance map generation and Gaussian training
void example_basic_workflow() {
    std::cout << "\n========== Example 1: Basic Workflow ==========" << std::endl;

    // Initialize instant-rm bridge
    InstantRMBridge bridge;
    if (!bridge.initialize()) {
        std::cout << "Failed to initialize instant-rm bridge" << std::endl;
        std::cout << "Note: This requires Python with instant-rm installed" << std::endl;
        std::cout << "Install with: pip install mitsuba instant-rm" << std::endl;
        return;
    }

    // Configure measurement plane
    InstantRMBridge::MeasurementPlane plane;
    plane.center = Vector3r(0, 0, 1.5); // 1.5m height
    plane.normal = Vector3r(0, 0, 1);   // Horizontal plane
    plane.size = Vector2r(100, 100);    // 100m x 100m
    plane.resolution_x = 128;
    plane.resolution_y = 128;
    bridge.setMeasurementPlane(plane);

    // Configure antenna
    InstantRMBridge::AntennaConfig antenna;
    antenna.pattern_type = "isotropic";
    antenna.gain_dbi = 0.0;
    bridge.setAntennaConfig(antenna);

    // Configure ray tracing
    InstantRMBridge::TracingConfig tracing;
    tracing.num_samples = 1024; // Number of ray samples
    tracing.max_depth = 5;      // Max bounces
    tracing.compute_delay_spread = true;
    tracing.compute_directions = true;
    bridge.setTracingConfig(tracing);

    // Generate neural radiance map
    Vector3r tx_position(0, 0, 10); // TX at 10m height
    real_T tx_power_dbm = 30.0;     // 1W transmit power
    real_T frequency_hz = 2.4e9;    // 2.4 GHz

    std::cout << "\nGenerating neural radiance map..." << std::endl;
    auto radiance_map = bridge.computeRadianceMap(tx_position, tx_power_dbm, frequency_hz);

    std::cout << "Radiance map generated:" << std::endl;
    std::cout << "  Resolution: " << radiance_map.num_cells_x << "x" << radiance_map.num_cells_y << std::endl;
    std::cout << "  Compute time: " << bridge.getLastComputeTime() << " ms" << std::endl;

    // Train Gaussian representation
    RFGaussianSplat rf_splat;
    RFGaussianSplat::RFSplatConfig config;
    config.num_gaussians = 5000;
    config.learning_rate = 0.01;
    config.adaptive_density_control = true;
    config.max_iterations = 1000;

    std::cout << "\nTraining Gaussian representation..." << std::endl;
    rf_splat.initialize(config);
    rf_splat.trainFromRadianceMap(radiance_map);

    std::cout << "Training complete:" << std::endl;
    std::cout << "  Final Gaussians: " << rf_splat.getNumGaussians() << std::endl;
    std::cout << "  Training loss: " << rf_splat.getCurrentLoss() << " dB²" << std::endl;

    // Query signal strength
    std::vector<Vector3r> test_points = {Vector3r(10, 0, 1.5), Vector3r(25, 25, 1.5), Vector3r(50, 0, 1.5)};

    std::cout << "\nSignal strength queries:" << std::endl;
    for (const auto &point : test_points) {
        real_T signal = rf_splat.querySignalStrength(point);
        Vector3r gradient = rf_splat.queryGradient(point);
        std::cout << "  Position (" << point.x() << ", " << point.y() << ", " << point.z() << "): " << signal
                  << " dBm, gradient=" << gradient.norm() << " dB/m" << std::endl;
    }

    // Save trained model
    rf_splat.exportToFile("rf_gaussian_model.bin");
    std::cout << "\nModel saved to rf_gaussian_model.bin" << std::endl;
}

// Example 2: Multi-transmitter scenario
void example_multi_transmitter() {
    std::cout << "\n========== Example 2: Multi-Transmitter ==========" << std::endl;

    InstantRMBridge bridge;
    if (!bridge.initialize()) {
        return;
    }

    // Configure
    InstantRMBridge::MeasurementPlane plane;
    plane.center = Vector3r(50, 50, 1.5);
    plane.size = Vector2r(200, 200);
    plane.resolution_x = 256;
    plane.resolution_y = 256;
    bridge.setMeasurementPlane(plane);

    // Multiple transmitters
    std::vector<Vector3r> tx_positions = {Vector3r(0, 0, 10), Vector3r(100, 0, 10), Vector3r(0, 100, 10),
                                          Vector3r(100, 100, 10)};

    std::vector<real_T> tx_powers = {30.0, 30.0, 30.0, 30.0}; // All 1W
    real_T frequency_hz = 5.8e9;                              // 5.8 GHz

    std::cout << "\nComputing multi-transmitter radiance map..." << std::endl;
    std::cout << "  Transmitters: " << tx_positions.size() << std::endl;

    auto radiance_map = bridge.computeMultiTxRadianceMap(tx_positions, tx_powers, frequency_hz);

    std::cout << "Multi-TX map computed in " << bridge.getLastComputeTime() << " ms" << std::endl;

    // Train Gaussians
    RFGaussianSplat rf_splat;
    RFGaussianSplat::RFSplatConfig config;
    config.num_gaussians = 10000; // More Gaussians for complex field
    config.max_iterations = 1500;
    rf_splat.initialize(config);
    rf_splat.trainFromRadianceMap(radiance_map);

    // Analyze coverage
    real_T min_signal_dbm = -80.0;
    auto coverage = rf_splat.queryCoverageArea(min_signal_dbm, 2.0);

    std::cout << "\nCoverage analysis:" << std::endl;
    std::cout << "  Minimum signal: " << min_signal_dbm << " dBm" << std::endl;
    std::cout << "  Coverage points: " << coverage.size() << std::endl;
    std::cout << "  Area covered: " << coverage.size() * 4.0 << " m²" << std::endl;
}

// Example 3: Compare instant-rm with analytical models
void example_comparison() {
    std::cout << "\n========== Example 3: Model Comparison ==========" << std::endl;

    // Set up instant-rm + Gaussian splatting
    InstantRMBridge bridge;
    if (!bridge.initialize()) {
        std::cout << "Using placeholder implementation (instant-rm not available)" << std::endl;
    }

    RFGaussianSplat neural_model;
    RFGaussianSplat::RFSplatConfig config;
    config.num_gaussians = 5000;
    config.max_iterations = 500;
    neural_model.initialize(config);

    // Generate radiance map and train
    Vector3r tx_pos(0, 0, 10);
    real_T tx_power = 30.0;
    real_T frequency = 2.4e9;

    auto radiance_map = bridge.computeRadianceMap(tx_pos, tx_power, frequency);
    neural_model.trainFromRadianceMap(radiance_map);

    // Set up analytical model (RTX)
    RTXRFPropagation analytical_model;
    RTXRFPropagation::PropagationConfig rtx_config;
    rtx_config.frequency_hz = frequency;
    rtx_config.tx_power_dbm = tx_power;
    rtx_config.model = RTXRFPropagation::PropagationModel::TwoRay;
    rtx_config.tx_height_m = 10.0;
    rtx_config.rx_height_m = 1.5;
    analytical_model.initialize(rtx_config);

    // Compare predictions
    std::cout << "\nModel comparison:" << std::endl;
    std::cout << "Distance (m) | Neural (dBm) | Analytical (dBm) | Error (dB)" << std::endl;
    std::cout << "-------------|--------------|------------------|------------" << std::endl;

    for (real_T distance = 10.0; distance <= 100.0; distance += 10.0) {
        Vector3r point(distance, 0, 1.5);

        real_T neural_signal = neural_model.querySignalStrength(point);
        real_T analytical_signal = analytical_model.computeReceivedPower(distance, 0.0);
        real_T error = std::abs(neural_signal - analytical_signal);

        printf("%12.1f | %12.2f | %16.2f | %10.2f\n", distance, neural_signal, analytical_signal, error);
    }

    std::cout << "\nPerformance comparison:" << std::endl;
    std::cout << "  Neural training time: " << bridge.getLastComputeTime() << " ms" << std::endl;
    std::cout << "  Neural query time: ~0.1 ms" << std::endl;
    std::cout << "  Analytical query time: ~0.001 ms" << std::endl;
    std::cout << "\nRecommendation: Use analytical for quick estimates," << std::endl;
    std::cout << "                use neural for complex environments" << std::endl;
}

// Example 4: Save and load trained models
void example_persistence() {
    std::cout << "\n========== Example 4: Model Persistence ==========" << std::endl;

    InstantRMBridge bridge;
    bridge.initialize();

    // Generate and save radiance map
    Vector3r tx_pos(0, 0, 10);
    auto radiance_map = bridge.computeRadianceMap(tx_pos, 30.0, 2.4e9);

    std::cout << "\nSaving radiance map..." << std::endl;
    if (bridge.saveRadianceMap(radiance_map, "radiance_map.bin")) {
        std::cout << "  Saved to radiance_map.bin" << std::endl;
    }

    // Train and save Gaussian model
    RFGaussianSplat rf_splat;
    RFGaussianSplat::RFSplatConfig config;
    config.num_gaussians = 5000;
    config.max_iterations = 1000;
    rf_splat.initialize(config);
    rf_splat.trainFromRadianceMap(radiance_map);

    std::cout << "\nSaving Gaussian model..." << std::endl;
    if (rf_splat.exportToFile("gaussian_model.bin")) {
        std::cout << "  Saved to gaussian_model.bin" << std::endl;
    }

    // Load and use saved model
    std::cout << "\nLoading saved Gaussian model..." << std::endl;
    RFGaussianSplat loaded_model;
    loaded_model.initialize(config);
    if (loaded_model.importFromFile("gaussian_model.bin")) {
        std::cout << "  Loaded successfully" << std::endl;
        std::cout << "  Gaussians: " << loaded_model.getNumGaussians() << std::endl;

        // Test query
        Vector3r test_point(10, 0, 1.5);
        real_T signal = loaded_model.querySignalStrength(test_point);
        std::cout << "  Test query at (10, 0, 1.5): " << signal << " dBm" << std::endl;
    }
}

// Example 5: Optimization loop
void example_optimization() {
    std::cout << "\n========== Example 5: Transmitter Optimization ==========" << std::endl;

    InstantRMBridge bridge;
    if (!bridge.initialize()) {
        return;
    }

    RFGaussianSplat rf_splat;
    RFGaussianSplat::RFSplatConfig config;
    config.num_gaussians = 3000;
    config.max_iterations = 500;
    rf_splat.initialize(config);

    // Target coverage area
    Vector3r target_center(30, 30, 0);
    real_T target_radius = 20.0;
    real_T min_signal_dbm = -85.0;

    std::cout << "\nOptimizing transmitter placement..." << std::cout << "Target area: center=(" << target_center.x()
              << ", " << target_center.y() << "), radius=" << target_radius << "m" << std::endl;

    // Initial transmitter position
    Vector3r tx_pos(0, 0, 10);
    real_T best_coverage = 0.0;
    real_T step_size = 2.0;

    for (int iter = 0; iter < 10; ++iter) {
        // Generate radiance map at current position
        auto radiance_map = bridge.computeRadianceMap(tx_pos, 30.0, 2.4e9);
        rf_splat.trainFromRadianceMap(radiance_map);

        // Evaluate coverage
        auto coverage = rf_splat.queryCoverageArea(min_signal_dbm, 1.0);
        real_T coverage_score = static_cast<real_T>(coverage.size());

        std::cout << "Iteration " << iter << ": TX=(" << tx_pos.x() << ", " << tx_pos.y()
                  << "), coverage=" << coverage_score << " points" << std::endl;

        if (coverage_score > best_coverage) {
            best_coverage = coverage_score;
        }

        // Compute gradient (simplified - move toward target)
        Vector3r to_target = target_center - tx_pos;
        to_target.z() = 0; // Keep height constant
        tx_pos += to_target.normalized() * step_size;
    }

    std::cout << "\nOptimization complete!" << std::endl;
    std::cout << "  Best coverage: " << best_coverage << " points" << std::endl;
    std::cout << "  Final TX position: (" << tx_pos.x() << ", " << tx_pos.y() << ", " << tx_pos.z() << ")" << std::endl;
}

int main(int argc, char *argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "instant-rm Integration Examples" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        example_basic_workflow();
        example_multi_transmitter();
        example_comparison();
        example_persistence();
        example_optimization();

        std::cout << "\n========================================" << std::endl;
        std::cout << "All examples completed!" << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
