#ifndef AUTONOMYSIM_H
#define AUTONOMYSIM_H

#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Result codes for C API
 */
typedef enum AutonomySimResult {
  /**
   * Operation succeeded
   */
  Ok = 0,
  /**
   * Null pointer passed
   */
  NullPointer = 1,
  /**
   * Invalid parameter
   */
  InvalidParameter = 2,
  /**
   * Training failed
   */
  TrainingFailed = 3,
  /**
   * Query failed
   */
  QueryFailed = 4,
  /**
   * Memory allocation failed
   */
  OutOfMemory = 5,
} AutonomySimResult;

/**
 * 3D vector for positions and directions
 *
 * Compatible with most C/C++ vector libraries.
 */
typedef struct AutonomySimVec3 {
  double x;
  double y;
  double z;
} AutonomySimVec3;

/**
 * Configuration for Gaussian splatting training
 */
typedef struct AutonomySimGaussianConfig {
  uintptr_t num_gaussians;
  uintptr_t max_iterations;
  double convergence_threshold;
  double learning_rate;
  double initial_variance;
  double regularization;
  bool parallel;
} AutonomySimGaussianConfig;

/**
 * RF measurement data point
 */
typedef struct AutonomySimRFMeasurement {
  struct AutonomySimVec3 position;
  double rssi_dbm;
  double weight;
} AutonomySimRFMeasurement;

/**
 * Opaque handle to Gaussian RF field
 *
 * Must be freed with `autonomysim_gaussian_field_free()`
 */
typedef struct AutonomySimGaussianField {
  uint8_t _private[0];
} AutonomySimGaussianField;

/**
 * Calculate free space path loss (Friis equation)
 *
 * # Arguments
 *
 * * `tx` - Transmitter position (meters)
 * * `rx` - Receiver position (meters)
 * * `frequency_hz` - Frequency in Hz
 *
 * # Returns
 *
 * Path loss in dB
 *
 * # Safety
 *
 * This function is safe to call from C.
 */
double autonomysim_free_space_path_loss(struct AutonomySimVec3 tx,
                                        struct AutonomySimVec3 rx,
                                        double frequency_hz);

/**
 * Calculate log-distance path loss
 *
 * # Arguments
 *
 * * `tx` - Transmitter position (meters)
 * * `rx` - Receiver position (meters)
 * * `frequency_hz` - Frequency in Hz
 * * `path_loss_exponent` - Path loss exponent (typically 2-4)
 * * `reference_distance` - Reference distance in meters (typically 1.0)
 *
 * # Returns
 *
 * Path loss in dB
 */
double autonomysim_log_distance_path_loss(struct AutonomySimVec3 tx,
                                          struct AutonomySimVec3 rx,
                                          double frequency_hz,
                                          double path_loss_exponent,
                                          double reference_distance);

/**
 * Calculate two-ray ground reflection path loss
 *
 * # Arguments
 *
 * * `tx` - Transmitter position (meters)
 * * `rx` - Receiver position (meters)
 * * `frequency_hz` - Frequency in Hz
 *
 * # Returns
 *
 * Path loss in dB including ground reflection
 */
double autonomysim_two_ray_ground_reflection(struct AutonomySimVec3 tx,
                                             struct AutonomySimVec3 rx,
                                             double frequency_hz);

/**
 * Calculate link budget
 *
 * # Arguments
 *
 * * `tx_power_dbm` - Transmitter power in dBm
 * * `tx_gain_db` - Transmitter antenna gain in dB
 * * `rx_gain_db` - Receiver antenna gain in dB
 * * `path_loss_db` - Path loss in dB
 * * `additional_losses_db` - Additional losses (cable, etc.) in dB
 *
 * # Returns
 *
 * Received signal strength in dBm
 */
double autonomysim_link_budget(double tx_power_dbm,
                               double tx_gain_db,
                               double rx_gain_db,
                               double path_loss_db,
                               double additional_losses_db);

/**
 * Calculate wavelength from frequency
 *
 * # Arguments
 *
 * * `frequency_hz` - Frequency in Hz
 *
 * # Returns
 *
 * Wavelength in meters
 */
double autonomysim_wavelength(double frequency_hz);

/**
 * Convert dBm to watts
 *
 * # Arguments
 *
 * * `dbm` - Power in dBm
 *
 * # Returns
 *
 * Power in watts
 */
double autonomysim_dbm_to_watts(double dbm);

/**
 * Convert watts to dBm
 *
 * # Arguments
 *
 * * `watts` - Power in watts
 *
 * # Returns
 *
 * Power in dBm
 */
double autonomysim_watts_to_dbm(double watts);

/**
 * Calculate isotropic antenna gain (always 0 dB)
 *
 * # Returns
 *
 * Antenna gain in dBi (0.0)
 */
double autonomysim_isotropic_antenna_gain(void);

/**
 * Calculate directional antenna gain
 *
 * # Arguments
 *
 * * `direction` - Direction vector (will be normalized)
 * * `boresight` - Antenna boresight direction (will be normalized)
 * * `peak_gain_dbi` - Peak gain in dBi (unused - uses default 10 dBi)
 * * `beamwidth_degrees` - 3dB beamwidth in degrees (used for both az/el)
 *
 * # Returns
 *
 * Antenna gain in dBi (includes pattern loss)
 */
double autonomysim_directional_antenna_gain(struct AutonomySimVec3 direction,
                                            struct AutonomySimVec3 boresight,
                                            double _peak_gain_dbi,
                                            double beamwidth_degrees);

/**
 * Calculate dipole antenna gain
 *
 * # Arguments
 *
 * * `direction` - Direction vector (will be normalized)
 * * `dipole_axis` - Dipole axis direction (will be normalized)
 *
 * # Returns
 *
 * Antenna gain in dBi (includes pattern)
 */
double autonomysim_dipole_antenna_gain(struct AutonomySimVec3 direction,
                                       struct AutonomySimVec3 dipole_axis);

/**
 * Create default Gaussian training configuration
 *
 * # Returns
 *
 * Default configuration structure
 */
struct AutonomySimGaussianConfig autonomysim_gaussian_config_default(void);

/**
 * Train a Gaussian RF field from measurements
 *
 * # Arguments
 *
 * * `measurements` - Array of RF measurements
 * * `num_measurements` - Number of measurements in array
 * * `config` - Training configuration
 * * `out_field` - Output pointer for trained field (must be freed)
 *
 * # Returns
 *
 * Result code
 *
 * # Safety
 *
 * - `measurements` must point to valid array of size `num_measurements`
 * - `out_field` must be a valid pointer
 * - Returned field must be freed with `autonomysim_gaussian_field_free()`
 */
enum AutonomySimResult autonomysim_gaussian_field_train(const struct AutonomySimRFMeasurement *measurements,
                                                        uintptr_t num_measurements,
                                                        struct AutonomySimGaussianConfig config,
                                                        struct AutonomySimGaussianField **out_field);

/**
 * Query signal strength at a position
 *
 * # Arguments
 *
 * * `field` - Trained Gaussian field
 * * `position` - Query position
 * * `out_rssi` - Output pointer for RSSI in dBm
 *
 * # Returns
 *
 * Result code
 *
 * # Safety
 *
 * - `field` must be a valid field returned from training
 * - `out_rssi` must be a valid pointer
 */
enum AutonomySimResult autonomysim_gaussian_field_query(const struct AutonomySimGaussianField *field,
                                                        struct AutonomySimVec3 position,
                                                        double *out_rssi);

/**
 * Query signal strength at multiple positions (batch)
 *
 * # Arguments
 *
 * * `field` - Trained Gaussian field
 * * `positions` - Array of query positions
 * * `num_positions` - Number of positions
 * * `out_rssi` - Output array for RSSI values (must be pre-allocated)
 *
 * # Returns
 *
 * Result code
 *
 * # Safety
 *
 * - `field` must be a valid field
 * - `positions` must point to valid array of size `num_positions`
 * - `out_rssi` must point to valid array of size `num_positions`
 */
enum AutonomySimResult autonomysim_gaussian_field_query_batch(const struct AutonomySimGaussianField *field,
                                                              const struct AutonomySimVec3 *positions,
                                                              uintptr_t num_positions,
                                                              double *out_rssi);

/**
 * Get number of Gaussians in the field
 *
 * # Arguments
 *
 * * `field` - Trained Gaussian field
 * * `out_count` - Output pointer for count
 *
 * # Returns
 *
 * Result code
 *
 * # Safety
 *
 * - `field` must be a valid field
 * - `out_count` must be a valid pointer
 */
enum AutonomySimResult autonomysim_gaussian_field_num_gaussians(const struct AutonomySimGaussianField *field,
                                                                uintptr_t *out_count);

/**
 * Free a Gaussian RF field
 *
 * # Arguments
 *
 * * `field` - Field to free (can be null)
 *
 * # Safety
 *
 * - `field` must be null or a valid field returned from training
 * - After calling, `field` pointer is invalid
 */
void autonomysim_gaussian_field_free(struct AutonomySimGaussianField *field);

/**
 * Get AutonomySim version string
 *
 * # Returns
 *
 * Null-terminated version string (valid for program lifetime)
 *
 * # Safety
 *
 * Returned pointer is valid for the lifetime of the program.
 */
const char *autonomysim_version(void);

#endif  /* AUTONOMYSIM_H */
