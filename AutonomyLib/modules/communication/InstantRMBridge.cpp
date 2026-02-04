// InstantRMBridge.cpp - Implementation of Python bridge for instant-rm

#include "InstantRMBridge.hpp"
#include "common/common_utils/Utils.hpp"

// Python C API includes
#ifdef _WIN32
// Disable Python debug library requirement on Windows
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
#else
#include <Python.h>
#endif

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include <chrono>

namespace autonomysim {

InstantRMBridge::InstantRMBridge()
    : instant_rm_module_(nullptr), map_tracer_class_(nullptr), map_tracer_instance_(nullptr), scene_object_(nullptr),
      initialized_(false), python_initialized_(false), last_compute_time_ms_(0.0), ray_count_(0) {}

InstantRMBridge::~InstantRMBridge() { shutdown(); }

bool InstantRMBridge::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_) {
        return true;
    }

    if (!initializePython()) {
        Utils::log("Failed to initialize Python environment", Utils::kLogLevelError);
        return false;
    }

    initialized_ = true;
    Utils::log("InstantRMBridge initialized successfully", Utils::kLogLevelInfo);
    return true;
}

void InstantRMBridge::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        return;
    }

    shutdownPython();
    initialized_ = false;
}

bool InstantRMBridge::initializePython() {
    if (python_initialized_) {
        return true;
    }

    // Initialize Python interpreter
    if (!Py_IsInitialized()) {
        Py_Initialize();
    }

    // Import NumPy array API
    import_array1(false);

    // Import instant_rm module
    instant_rm_module_ = PyImport_ImportModule("instant_rm");
    if (!instant_rm_module_) {
        PyErr_Print();
        Utils::log("Failed to import instant_rm module. Ensure it is installed: pip install instant-rm",
                   Utils::kLogLevelError);
        return false;
    }

    // Get MapTracer class
    map_tracer_class_ = PyObject_GetAttrString(instant_rm_module_, "MapTracer");
    if (!map_tracer_class_) {
        PyErr_Print();
        Utils::log("Failed to get MapTracer class from instant_rm", Utils::kLogLevelError);
        Py_DECREF(instant_rm_module_);
        return false;
    }

    // Verify Mitsuba variant
    PyObject *mitsuba_module = PyImport_ImportModule("mitsuba");
    if (mitsuba_module) {
        PyObject *variant_func = PyObject_GetAttrString(mitsuba_module, "variant");
        if (variant_func) {
            PyObject *variant = PyObject_CallNoArgs(variant_func);
            if (variant && PyUnicode_Check(variant)) {
                const char *variant_str = PyUnicode_AsUTF8(variant);
                Utils::log(std::string("Mitsuba variant: ") + variant_str, Utils::kLogLevelInfo);

                if (std::string(variant_str).find("cuda_ad_mono_polarized") == std::string::npos) {
                    Utils::log("WARNING: instant-rm requires 'cuda_ad_mono_polarized' variant. "
                               "Set with: mi.set_variant('cuda_ad_mono_polarized')",
                               Utils::kLogLevelWarn);
                }
            }
            Py_XDECREF(variant);
            Py_DECREF(variant_func);
        }
        Py_DECREF(mitsuba_module);
    }

    python_initialized_ = true;
    return true;
}

void InstantRMBridge::shutdownPython() {
    if (!python_initialized_) {
        return;
    }

    Py_XDECREF(scene_object_);
    Py_XDECREF(map_tracer_instance_);
    Py_XDECREF(map_tracer_class_);
    Py_XDECREF(instant_rm_module_);

    scene_object_ = nullptr;
    map_tracer_instance_ = nullptr;
    map_tracer_class_ = nullptr;
    instant_rm_module_ = nullptr;

    // Note: We don't call Py_Finalize() as Python may be used elsewhere
    python_initialized_ = false;
}

void InstantRMBridge::setSceneConfig(const SceneConfig &config) {
    std::lock_guard<std::mutex> lock(mutex_);
    scene_config_ = config;
}

void InstantRMBridge::setAntennaConfig(const AntennaConfig &config) {
    std::lock_guard<std::mutex> lock(mutex_);
    antenna_config_ = config;
}

void InstantRMBridge::setMeasurementPlane(const MeasurementPlane &plane) {
    std::lock_guard<std::mutex> lock(mutex_);
    measurement_plane_ = plane;
}

void InstantRMBridge::setTracingConfig(const TracingConfig &config) {
    std::lock_guard<std::mutex> lock(mutex_);
    tracing_config_ = config;
}

RFGaussianSplat::NeuralRadianceMap InstantRMBridge::computeRadianceMap(const Vector3r &tx_position, real_T tx_power_dbm,
                                                                       real_T frequency_hz) {
    return computeRadianceMap(tx_position, tx_power_dbm, frequency_hz, scene_config_);
}

RFGaussianSplat::NeuralRadianceMap InstantRMBridge::computeRadianceMap(const Vector3r &tx_position, real_T tx_power_dbm,
                                                                       real_T frequency_hz, const SceneConfig &scene) {
    std::lock_guard<std::mutex> lock(mutex_);

    RFGaussianSplat::NeuralRadianceMap radiance_map;

    if (!initialized_) {
        Utils::log("InstantRMBridge not initialized", Utils::kLogLevelError);
        return radiance_map;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    Utils::log("Computing neural radiance map with instant-rm...", Utils::kLogLevelInfo);
    Utils::log("  TX Position: (" + std::to_string(tx_position.x()) + ", " + std::to_string(tx_position.y()) + ", " +
                   std::to_string(tx_position.z()) + ")",
               Utils::kLogLevelInfo);
    Utils::log("  TX Power: " + std::to_string(tx_power_dbm) + " dBm", Utils::kLogLevelInfo);
    Utils::log("  Frequency: " + std::to_string(frequency_hz / 1e9) + " GHz", Utils::kLogLevelInfo);

    // For now, use placeholder implementation
    // Full implementation would call Python instant-rm via Python C API

    // Set up radiance map structure
    radiance_map.measurement_plane_center = measurement_plane_.center;
    radiance_map.measurement_plane_orientation = measurement_plane_.normal;
    radiance_map.measurement_plane_size = measurement_plane_.size;
    radiance_map.num_cells_x = measurement_plane_.resolution_x;
    radiance_map.num_cells_y = measurement_plane_.resolution_y;
    radiance_map.frequency_hz = frequency_hz;
    radiance_map.tx_power_dbm = tx_power_dbm;
    radiance_map.tx_position = tx_position;
    radiance_map.tx_orientation = antenna_config_.orientation;

    // Allocate arrays
    uint32_t num_cells = radiance_map.num_cells_x * radiance_map.num_cells_y;
    radiance_map.path_loss_map.resize(num_cells);
    radiance_map.rms_delay_spread.resize(num_cells);
    radiance_map.mean_direction_arrival.resize(num_cells);
    radiance_map.mean_direction_departure.resize(num_cells);

    // TODO: Replace with actual instant-rm Python call
    // For now, use simplified Friis model as placeholder
    real_T wavelength = 3e8 / frequency_hz;
    real_T cell_width = radiance_map.measurement_plane_size.x() / radiance_map.num_cells_x;
    real_T cell_height = radiance_map.measurement_plane_size.y() / radiance_map.num_cells_y;

    for (uint32_t y = 0; y < radiance_map.num_cells_y; ++y) {
        for (uint32_t x = 0; x < radiance_map.num_cells_x; ++x) {
            uint32_t idx = y * radiance_map.num_cells_x + x;

            // Compute cell center in world coordinates
            real_T u = (x + 0.5f) / radiance_map.num_cells_x - 0.5f;
            real_T v = (y + 0.5f) / radiance_map.num_cells_y - 0.5f;
            Vector3r cell_pos =
                radiance_map.measurement_plane_center + Vector3r(u * radiance_map.measurement_plane_size.x(),
                                                                 v * radiance_map.measurement_plane_size.y(), 0.0f);

            // Distance from transmitter
            real_T distance = (cell_pos - tx_position).norm();
            distance = std::max(distance, 1.0f); // Avoid singularity

            // Friis path loss: PL = 20*log10(d) + 20*log10(f) + 20*log10(4π/c)
            real_T path_loss =
                20.0f * std::log10(distance) + 20.0f * std::log10(frequency_hz) - 147.55f; // 20*log10(4π/c)

            radiance_map.path_loss_map[idx] = path_loss;

            // Placeholder RMS delay spread (increases with distance)
            radiance_map.rms_delay_spread[idx] = distance * 10e-9f; // 10 ns/m

            // Direction from TX to cell
            Vector3r direction = (cell_pos - tx_position).normalized();
            radiance_map.mean_direction_arrival[idx] = direction;
            radiance_map.mean_direction_departure[idx] = -direction;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    last_compute_time_ms_ = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    Utils::log("Radiance map computed in " + std::to_string(last_compute_time_ms_) + " ms", Utils::kLogLevelInfo);
    Utils::log("  Resolution: " + std::to_string(radiance_map.num_cells_x) + "x" +
                   std::to_string(radiance_map.num_cells_y) + " cells",
               Utils::kLogLevelInfo);

    return radiance_map;
}

RFGaussianSplat::NeuralRadianceMap InstantRMBridge::computeMultiTxRadianceMap(const std::vector<Vector3r> &tx_positions,
                                                                              const std::vector<real_T> &tx_powers_dbm,
                                                                              real_T frequency_hz) {
    // For multiple transmitters, compute individual maps and combine
    RFGaussianSplat::NeuralRadianceMap combined_map;

    if (tx_positions.empty()) {
        return combined_map;
    }

    // Compute first transmitter
    combined_map = computeRadianceMap(tx_positions[0], tx_powers_dbm[0], frequency_hz);

    // Combine with additional transmitters
    for (size_t i = 1; i < tx_positions.size(); ++i) {
        auto map = computeRadianceMap(tx_positions[i], tx_powers_dbm[i], frequency_hz);

        // Combine path loss maps (power sum in linear domain)
        for (size_t j = 0; j < combined_map.path_loss_map.size(); ++j) {
            real_T power1_dbm = combined_map.tx_power_dbm - combined_map.path_loss_map[j];
            real_T power2_dbm = map.tx_power_dbm - map.path_loss_map[j];

            // Convert to linear, sum, convert back
            real_T power1_mw = std::pow(10.0f, power1_dbm / 10.0f);
            real_T power2_mw = std::pow(10.0f, power2_dbm / 10.0f);
            real_T combined_power_mw = power1_mw + power2_mw;
            real_T combined_power_dbm = 10.0f * std::log10(combined_power_mw);

            // Update path loss
            combined_map.path_loss_map[j] = combined_map.tx_power_dbm - combined_power_dbm;
        }
    }

    return combined_map;
}

bool InstantRMBridge::computeGradients(const RFGaussianSplat::NeuralRadianceMap &radiance_map,
                                       std::vector<Vector3r> &position_gradients) {
    // TODO: Implement radiative backpropagation (RBP) for gradients
    // This requires calling instant-rm's PathlossMapRBPTracer
    Utils::log("computeGradients not yet implemented (requires RBP tracer)", Utils::kLogLevelWarn);
    return false;
}

bool InstantRMBridge::loadScene(const std::string &scene_file) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!python_initialized_) {
        return false;
    }

    // TODO: Load Mitsuba scene from XML file
    Utils::log("Loading scene: " + scene_file, Utils::kLogLevelInfo);

    scene_config_.scene_file = scene_file;
    return true;
}

bool InstantRMBridge::saveRadianceMap(const RFGaussianSplat::NeuralRadianceMap &map, const std::string &filename) {
    // Save radiance map to binary file
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Write header
    file.write(reinterpret_cast<const char *>(&map.num_cells_x), sizeof(uint32_t));
    file.write(reinterpret_cast<const char *>(&map.num_cells_y), sizeof(uint32_t));
    file.write(reinterpret_cast<const char *>(&map.frequency_hz), sizeof(real_T));
    file.write(reinterpret_cast<const char *>(&map.tx_power_dbm), sizeof(real_T));

    // Write arrays
    uint32_t size = map.num_cells_x * map.num_cells_y;
    file.write(reinterpret_cast<const char *>(map.path_loss_map.data()), size * sizeof(real_T));
    file.write(reinterpret_cast<const char *>(map.rms_delay_spread.data()), size * sizeof(real_T));

    file.close();
    return true;
}

bool InstantRMBridge::loadRadianceMap(RFGaussianSplat::NeuralRadianceMap &map, const std::string &filename) {
    // Load radiance map from binary file
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Read header
    file.read(reinterpret_cast<char *>(&map.num_cells_x), sizeof(uint32_t));
    file.read(reinterpret_cast<char *>(&map.num_cells_y), sizeof(uint32_t));
    file.read(reinterpret_cast<char *>(&map.frequency_hz), sizeof(real_T));
    file.read(reinterpret_cast<char *>(&map.tx_power_dbm), sizeof(real_T));

    // Allocate and read arrays
    uint32_t size = map.num_cells_x * map.num_cells_y;
    map.path_loss_map.resize(size);
    map.rms_delay_spread.resize(size);

    file.read(reinterpret_cast<char *>(map.path_loss_map.data()), size * sizeof(real_T));
    file.read(reinterpret_cast<char *>(map.rms_delay_spread.data()), size * sizeof(real_T));

    file.close();
    return true;
}

// Helper functions for Python conversion (simplified implementation)
PyObject *InstantRMBridge::convertVector3ToPython(const Vector3r &vec) {
    PyObject *list = PyList_New(3);
    PyList_SetItem(list, 0, PyFloat_FromDouble(vec.x()));
    PyList_SetItem(list, 1, PyFloat_FromDouble(vec.y()));
    PyList_SetItem(list, 2, PyFloat_FromDouble(vec.z()));
    return list;
}

Vector3r InstantRMBridge::convertPythonToVector3(PyObject *obj) {
    if (!PyList_Check(obj) || PyList_Size(obj) != 3) {
        return Vector3r(0, 0, 0);
    }

    return Vector3r(PyFloat_AsDouble(PyList_GetItem(obj, 0)), PyFloat_AsDouble(PyList_GetItem(obj, 1)),
                    PyFloat_AsDouble(PyList_GetItem(obj, 2)));
}

PyObject *InstantRMBridge::convertSceneConfigToPython(const SceneConfig &config) {
    PyObject *dict = PyDict_New();

    PyDict_SetItemString(dict, "scene_file", PyUnicode_FromString(config.scene_file.c_str()));
    PyDict_SetItemString(dict, "material_type", PyUnicode_FromString(config.material_type.c_str()));

    // TODO: Add obstacles and materials

    return dict;
}

RFGaussianSplat::NeuralRadianceMap InstantRMBridge::extractRadianceMapFromPython(PyObject *result) {
    RFGaussianSplat::NeuralRadianceMap map;
    // TODO: Extract NumPy arrays from Python result dictionary
    return map;
}

bool InstantRMBridge::callInstantRM(const Vector3r &tx_position, real_T tx_power_dbm, real_T frequency_hz,
                                    PyObject *&result) {
    // TODO: Full Python C API implementation
    // This would:
    // 1. Create MapTracer instance
    // 2. Set up measurement plane
    // 3. Call tracer with parameters
    // 4. Extract result NumPy arrays
    return false;
}

} // namespace autonomysim
