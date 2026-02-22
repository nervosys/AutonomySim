//! Spatial partitioning for distributed simulation

use nalgebra::Vector3;
use serde::{Deserialize, Serialize};

/// 3D spatial partition for agent assignment
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Partition {
    pub id: usize,
    pub min_bounds: Vector3<f64>,
    pub max_bounds: Vector3<f64>,
    pub neighbor_partitions: Vec<usize>,
}

impl Partition {
    /// Check if point is inside partition
    pub fn contains(&self, point: &Vector3<f64>) -> bool {
        point.x >= self.min_bounds.x
            && point.x < self.max_bounds.x
            && point.y >= self.min_bounds.y
            && point.y < self.max_bounds.y
            && point.z >= self.min_bounds.z
            && point.z < self.max_bounds.z
    }

    /// Get center of partition
    pub fn center(&self) -> Vector3<f64> {
        (self.min_bounds + self.max_bounds) / 2.0
    }

    /// Get volume of partition
    pub fn volume(&self) -> f64 {
        let dims = self.max_bounds - self.min_bounds;
        dims.x * dims.y * dims.z
    }
}

/// Spatial partitioner for dividing simulation space
pub struct SpatialPartitioner {
    partitions: Vec<Partition>,
    _bounds: Vector3<f64>,
}

impl SpatialPartitioner {
    /// Create new spatial partitioner with grid-based partitions
    pub fn new(bounds: Vector3<f64>, num_partitions: usize) -> Self {
        let partitions = Self::create_grid_partitions(bounds, num_partitions);
        Self {
            partitions,
            _bounds: bounds,
        }
    }

    /// Create grid-based partitions (e.g., 2x2x1 for 4 partitions)
    fn create_grid_partitions(bounds: Vector3<f64>, num_partitions: usize) -> Vec<Partition> {
        // For simplicity, divide along X and Y axes
        // For 4 partitions: 2x2x1 grid
        // For 8 partitions: 2x2x2 grid
        // For 16 partitions: 4x4x1 grid

        let (nx, ny, nz) = Self::compute_grid_dimensions(num_partitions);
        let dx = bounds.x / nx as f64;
        let dy = bounds.y / ny as f64;
        let dz = bounds.z / nz as f64;

        let mut partitions = Vec::new();
        let mut id = 0;

        for ix in 0..nx {
            for iy in 0..ny {
                for iz in 0..nz {
                    let min_bounds = Vector3::new(ix as f64 * dx, iy as f64 * dy, iz as f64 * dz);

                    let max_bounds = Vector3::new(
                        (ix + 1) as f64 * dx,
                        (iy + 1) as f64 * dy,
                        (iz + 1) as f64 * dz,
                    );

                    // Find neighbor partitions (share face)
                    let mut neighbors = Vec::new();

                    // +X neighbor
                    if ix + 1 < nx {
                        neighbors.push(Self::grid_to_id(ix + 1, iy, iz, nx, ny));
                    }
                    // -X neighbor
                    if ix > 0 {
                        neighbors.push(Self::grid_to_id(ix - 1, iy, iz, nx, ny));
                    }
                    // +Y neighbor
                    if iy + 1 < ny {
                        neighbors.push(Self::grid_to_id(ix, iy + 1, iz, nx, ny));
                    }
                    // -Y neighbor
                    if iy > 0 {
                        neighbors.push(Self::grid_to_id(ix, iy - 1, iz, nx, ny));
                    }
                    // +Z neighbor
                    if iz + 1 < nz {
                        neighbors.push(Self::grid_to_id(ix, iy, iz + 1, nx, ny));
                    }
                    // -Z neighbor
                    if iz > 0 {
                        neighbors.push(Self::grid_to_id(ix, iy, iz - 1, nx, ny));
                    }

                    partitions.push(Partition {
                        id,
                        min_bounds,
                        max_bounds,
                        neighbor_partitions: neighbors,
                    });

                    id += 1;
                }
            }
        }

        partitions
    }

    /// Compute grid dimensions for N partitions
    fn compute_grid_dimensions(num_partitions: usize) -> (usize, usize, usize) {
        match num_partitions {
            1 => (1, 1, 1),
            2 => (2, 1, 1),
            4 => (2, 2, 1),
            8 => (2, 2, 2),
            16 => (4, 4, 1),
            32 => (4, 4, 2),
            64 => (4, 4, 4),
            _ => {
                // For arbitrary N, approximate cube root
                let n_cube = (num_partitions as f64).cbrt().ceil() as usize;
                (n_cube, n_cube, n_cube)
            }
        }
    }

    /// Convert grid coordinates to partition ID
    fn grid_to_id(ix: usize, iy: usize, iz: usize, _nx: usize, ny: usize) -> usize {
        ix * ny * 1 + iy * 1 + iz // Assumes nz=1 for simplicity
    }

    /// Find which partition contains a point
    pub fn find_partition(&self, point: &Vector3<f64>) -> Option<usize> {
        self.partitions
            .iter()
            .find(|p| p.contains(point))
            .map(|p| p.id)
    }

    /// Get all partitions
    pub fn partitions(&self) -> &[Partition] {
        &self.partitions
    }

    /// Get specific partition
    pub fn partition(&self, id: usize) -> Option<&Partition> {
        self.partitions.get(id)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_partition_contains() {
        let partition = Partition {
            id: 0,
            min_bounds: Vector3::zeros(),
            max_bounds: Vector3::new(100.0, 100.0, 100.0),
            neighbor_partitions: vec![],
        };

        assert!(partition.contains(&Vector3::new(50.0, 50.0, 50.0)));
        assert!(!partition.contains(&Vector3::new(150.0, 50.0, 50.0)));
    }

    #[test]
    fn test_spatial_partitioner() {
        let bounds = Vector3::new(1000.0, 1000.0, 100.0);
        let partitioner = SpatialPartitioner::new(bounds, 4);

        assert_eq!(partitioner.partitions().len(), 4);

        // Test point assignment
        let point = Vector3::new(250.0, 250.0, 50.0);
        let partition_id = partitioner.find_partition(&point);
        assert!(partition_id.is_some());
    }

    #[test]
    fn test_partition_neighbors() {
        let bounds = Vector3::new(1000.0, 1000.0, 100.0);
        let partitioner = SpatialPartitioner::new(bounds, 4);

        // In 2x2 grid, partition 0 should have 2 neighbors (right and top)
        let partition = partitioner.partition(0).unwrap();
        assert!(!partition.neighbor_partitions.is_empty());
    }
}
