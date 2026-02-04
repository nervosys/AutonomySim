//! Native Rust backend for simulation
//!
//! This module implements a fast, CPU-based ray tracing backend using
//! pure Rust. It's designed for RF propagation simulation where complex
//! physics simulation is not needed, but fast ray casting is essential.
//!
//! Uses BVH (Bounding Volume Hierarchy) for efficient ray-mesh intersection.

use crate::backend::*;
use async_trait::async_trait;
use nalgebra::{Point3, Vector3};
use parking_lot::RwLock;
use std::collections::HashMap;
use std::sync::Arc;

use crate::sensor::{GpsData, ImuData, SensorData};
use crate::vehicle::{
    CollisionInfo, VehicleControl, VehicleId, VehicleSpec, VehicleState, VehicleType,
};

/// Native Rust backend implementation
pub struct NativeBackend {
    scenes: Arc<RwLock<HashMap<String, NativeScene>>>,
    vehicles: Arc<RwLock<HashMap<VehicleId, NativeVehicle>>>,
    time: f64,
    initialized: bool,
}

/// Vehicle representation in native backend
struct NativeVehicle {
    spec: VehicleSpec,
    state: VehicleState,
    control: VehicleControl,
}

impl NativeBackend {
    pub fn new() -> Self {
        Self {
            scenes: Arc::new(RwLock::new(HashMap::new())),
            vehicles: Arc::new(RwLock::new(HashMap::new())),
            time: 0.0,
            initialized: false,
        }
    }
}

#[async_trait]
impl SimulationBackend for NativeBackend {
    fn name(&self) -> &str {
        "Native Rust Backend"
    }

    async fn initialize(&mut self, _config: BackendConfig) -> SimResult<()> {
        self.initialized = true;
        Ok(())
    }

    async fn shutdown(&mut self) -> SimResult<()> {
        self.scenes.write().clear();
        self.initialized = false;
        Ok(())
    }

    fn is_initialized(&self) -> bool {
        self.initialized
    }

    async fn load_scene(&mut self, scene_path: &str) -> SimResult<SceneHandle> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "Backend must be initialized before loading scenes".to_string(),
            ));
        }

        let scene_id = format!("scene_{}", uuid::Uuid::new_v4());
        let scene = NativeScene::new(scene_path.to_string());

        self.scenes.write().insert(scene_id.clone(), scene);

        Ok(SceneHandle::new(scene_id, BackendType::Native))
    }

    fn get_scene_bounds(&self, scene: &SceneHandle) -> SimResult<(Position, Position)> {
        let scenes = self.scenes.read();
        let native_scene = scenes
            .get(&scene.id)
            .ok_or_else(|| SimError::SceneNotFound(scene.id.clone()))?;

        Ok(native_scene.get_bounds())
    }

    fn add_object(&mut self, scene: &SceneHandle, object: SceneObject) -> SimResult<String> {
        let mut scenes = self.scenes.write();
        let native_scene = scenes
            .get_mut(&scene.id)
            .ok_or_else(|| SimError::SceneNotFound(scene.id.clone()))?;

        Ok(native_scene.add_object(object))
    }

    fn remove_object(&mut self, scene: &SceneHandle, object_id: &str) -> SimResult<()> {
        let mut scenes = self.scenes.write();
        let native_scene = scenes
            .get_mut(&scene.id)
            .ok_or_else(|| SimError::SceneNotFound(scene.id.clone()))?;

        native_scene.remove_object(object_id)
    }

    fn update_transform(
        &mut self,
        scene: &SceneHandle,
        object_id: &str,
        transform: Transform,
    ) -> SimResult<()> {
        let mut scenes = self.scenes.write();
        let native_scene = scenes
            .get_mut(&scene.id)
            .ok_or_else(|| SimError::SceneNotFound(scene.id.clone()))?;

        native_scene.update_transform(object_id, transform)
    }

    fn cast_ray(&self, scene: &SceneHandle, ray: &Ray) -> SimResult<Option<RayHit>> {
        let scenes = self.scenes.read();
        let native_scene = scenes
            .get(&scene.id)
            .ok_or_else(|| SimError::SceneNotFound(scene.id.clone()))?;

        Ok(native_scene.cast_ray(ray))
    }

    fn cast_rays(&self, scene: &SceneHandle, rays: &[Ray]) -> SimResult<Vec<Option<RayHit>>> {
        let scenes = self.scenes.read();
        let native_scene = scenes
            .get(&scene.id)
            .ok_or_else(|| SimError::SceneNotFound(scene.id.clone()))?;

        Ok(native_scene.cast_rays(rays))
    }

    fn get_objects(&self, scene: &SceneHandle) -> SimResult<Vec<SceneObject>> {
        let scenes = self.scenes.read();
        let native_scene = scenes
            .get(&scene.id)
            .ok_or_else(|| SimError::SceneNotFound(scene.id.clone()))?;

        Ok(native_scene.get_objects())
    }

    async fn step(&mut self, delta_time: f64) -> SimResult<()> {
        self.time += delta_time;
        Ok(())
    }

    fn get_time(&self) -> f64 {
        self.time
    }

    async fn spawn_vehicle(&mut self, spec: VehicleSpec) -> SimResult<VehicleId> {
        let vehicle_id = spec.vehicle_id.clone();

        let state = VehicleState {
            vehicle_id: vehicle_id.clone(),
            timestamp: self.time,
            transform: spec.initial_transform.clone(),
            linear_velocity: Vector3::zeros(),
            angular_velocity: Vector3::zeros(),
            linear_acceleration: Vector3::zeros(),
            angular_acceleration: Vector3::zeros(),
            battery_level: 1.0,
            is_grounded: false,
            collision_info: None,
        };

        let vehicle = NativeVehicle {
            spec,
            state,
            control: VehicleControl::default(),
        };

        self.vehicles.write().insert(vehicle_id.clone(), vehicle);
        Ok(vehicle_id)
    }

    async fn remove_vehicle(&mut self, vehicle_id: &str) -> SimResult<()> {
        self.vehicles
            .write()
            .remove(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?;
        Ok(())
    }

    fn get_vehicle_state(&self, vehicle_id: &str) -> SimResult<VehicleState> {
        let vehicles = self.vehicles.read();
        let vehicle = vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?;
        Ok(vehicle.state.clone())
    }

    fn set_vehicle_control(&mut self, vehicle_id: &str, control: VehicleControl) -> SimResult<()> {
        let mut vehicles = self.vehicles.write();
        let vehicle = vehicles
            .get_mut(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?;
        vehicle.control = control;
        Ok(())
    }

    fn get_sensor_data(&self, vehicle_id: &str, sensor_id: &str) -> SimResult<SensorData> {
        let vehicles = self.vehicles.read();
        let vehicle = vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?;

        // Generate synthetic sensor data based on vehicle state
        match sensor_id {
            "imu" => {
                let imu_data = ImuData {
                    timestamp: self.time,
                    linear_acceleration: vehicle.state.linear_acceleration,
                    angular_velocity: vehicle.state.angular_velocity,
                    orientation: vehicle.state.transform.rotation,
                };
                Ok(SensorData::Imu(imu_data))
            }
            "gps" => {
                let gps_data = GpsData {
                    timestamp: self.time,
                    latitude: vehicle.state.transform.position.x / 111320.0, // Approximate
                    longitude: vehicle.state.transform.position.y / 111320.0,
                    altitude: vehicle.state.transform.position.z,
                    velocity: vehicle.state.linear_velocity,
                    eph: 1.0,
                    epv: 1.5,
                    fix_type: crate::sensor::GpsFixType::Fix3D,
                };
                Ok(SensorData::Gps(gps_data))
            }
            _ => Err(SimError::BackendError(format!(
                "Sensor not found: {}",
                sensor_id
            ))),
        }
    }
}

/// Native scene representation
struct NativeScene {
    path: String,
    objects: HashMap<String, SceneObject>,
    bounds_min: Position,
    bounds_max: Position,
}

impl NativeScene {
    fn new(path: String) -> Self {
        Self {
            path,
            objects: HashMap::new(),
            bounds_min: Point3::new(f64::INFINITY, f64::INFINITY, f64::INFINITY),
            bounds_max: Point3::new(f64::NEG_INFINITY, f64::NEG_INFINITY, f64::NEG_INFINITY),
        }
    }

    fn get_bounds(&self) -> (Position, Position) {
        (self.bounds_min, self.bounds_max)
    }

    fn add_object(&mut self, object: SceneObject) -> String {
        let id = object.id.clone();
        self.update_bounds(&object);
        self.objects.insert(id.clone(), object);
        id
    }

    fn remove_object(&mut self, object_id: &str) -> SimResult<()> {
        self.objects
            .remove(object_id)
            .ok_or_else(|| SimError::BackendError(format!("Object not found: {}", object_id)))?;
        self.recompute_bounds();
        Ok(())
    }

    fn update_transform(&mut self, object_id: &str, transform: Transform) -> SimResult<()> {
        let object = self
            .objects
            .get_mut(object_id)
            .ok_or_else(|| SimError::BackendError(format!("Object not found: {}", object_id)))?;
        object.transform = transform;
        self.recompute_bounds();
        Ok(())
    }

    fn update_bounds(&mut self, object: &SceneObject) {
        let pos = &object.transform.position;
        self.bounds_min = Point3::new(
            self.bounds_min.x.min(pos.x),
            self.bounds_min.y.min(pos.y),
            self.bounds_min.z.min(pos.z),
        );
        self.bounds_max = Point3::new(
            self.bounds_max.x.max(pos.x),
            self.bounds_max.y.max(pos.y),
            self.bounds_max.z.max(pos.z),
        );
    }

    fn recompute_bounds(&mut self) {
        let mut new_min = Point3::new(f64::INFINITY, f64::INFINITY, f64::INFINITY);
        let mut new_max = Point3::new(f64::NEG_INFINITY, f64::NEG_INFINITY, f64::NEG_INFINITY);

        for object in self.objects.values() {
            let pos = &object.transform.position;
            new_min = Point3::new(
                new_min.x.min(pos.x),
                new_min.y.min(pos.y),
                new_min.z.min(pos.z),
            );
            new_max = Point3::new(
                new_max.x.max(pos.x),
                new_max.y.max(pos.y),
                new_max.z.max(pos.z),
            );
        }

        self.bounds_min = new_min;
        self.bounds_max = new_max;
    }

    fn cast_ray(&self, ray: &Ray) -> Option<RayHit> {
        let mut closest_hit: Option<RayHit> = None;
        let mut closest_distance = ray.max_distance;

        for object in self.objects.values() {
            if let Some(hit) = self.intersect_object(ray, object) {
                if hit.distance < closest_distance {
                    closest_distance = hit.distance;
                    closest_hit = Some(hit);
                }
            }
        }

        closest_hit
    }

    fn cast_rays(&self, rays: &[Ray]) -> Vec<Option<RayHit>> {
        rays.iter().map(|ray| self.cast_ray(ray)).collect()
    }

    fn intersect_object(&self, ray: &Ray, object: &SceneObject) -> Option<RayHit> {
        match &object.geometry {
            Geometry::Sphere { radius } => {
                self.intersect_sphere(ray, &object.transform.position, *radius, object)
            }
            Geometry::Box { size } => self.intersect_box(ray, &object.transform, size, object),
            Geometry::Cylinder { radius, height } => {
                self.intersect_cylinder(ray, &object.transform, *radius, *height, object)
            }
            Geometry::Mesh { vertices, indices } => {
                self.intersect_mesh(ray, &object.transform, vertices, indices, object)
            }
        }
    }

    fn intersect_sphere(
        &self,
        ray: &Ray,
        center: &Position,
        radius: f64,
        object: &SceneObject,
    ) -> Option<RayHit> {
        let oc = ray.origin - center;
        let a = ray.direction.dot(&ray.direction);
        let b = 2.0 * oc.dot(&ray.direction);
        let c = oc.dot(&oc) - radius * radius;
        let discriminant = b * b - 4.0 * a * c;

        if discriminant < 0.0 {
            return None;
        }

        let t = (-b - discriminant.sqrt()) / (2.0 * a);
        if t < 0.0 || t > ray.max_distance {
            return None;
        }

        let position = ray.at(t);
        let normal = ((position - center) / radius).normalize();

        Some(RayHit {
            distance: t,
            position,
            normal,
            material: object.material.clone(),
            object_id: object.id.clone(),
        })
    }

    fn intersect_box(
        &self,
        ray: &Ray,
        transform: &Transform,
        size: &Vec3,
        object: &SceneObject,
    ) -> Option<RayHit> {
        // AABB intersection (simplified - doesn't account for rotation)
        let min = transform.position - size / 2.0;
        let max = transform.position + size / 2.0;

        let mut tmin = (min.x - ray.origin.x) / ray.direction.x;
        let mut tmax = (max.x - ray.origin.x) / ray.direction.x;

        if tmin > tmax {
            std::mem::swap(&mut tmin, &mut tmax);
        }

        let mut tymin = (min.y - ray.origin.y) / ray.direction.y;
        let mut tymax = (max.y - ray.origin.y) / ray.direction.y;

        if tymin > tymax {
            std::mem::swap(&mut tymin, &mut tymax);
        }

        if tmin > tymax || tymin > tmax {
            return None;
        }

        if tymin > tmin {
            tmin = tymin;
        }
        if tymax < tmax {
            tmax = tymax;
        }

        let mut tzmin = (min.z - ray.origin.z) / ray.direction.z;
        let mut tzmax = (max.z - ray.origin.z) / ray.direction.z;

        if tzmin > tzmax {
            std::mem::swap(&mut tzmin, &mut tzmax);
        }

        if tmin > tzmax || tzmin > tmax {
            return None;
        }

        if tzmin > tmin {
            tmin = tzmin;
        }

        if tmin < 0.0 || tmin > ray.max_distance {
            return None;
        }

        let position = ray.at(tmin);
        let normal = self.compute_box_normal(&position, &min, &max);

        Some(RayHit {
            distance: tmin,
            position,
            normal,
            material: object.material.clone(),
            object_id: object.id.clone(),
        })
    }

    fn compute_box_normal(&self, point: &Position, min: &Position, max: &Position) -> Vec3 {
        let epsilon = 1e-6;
        let p = point;

        if (p.x - min.x).abs() < epsilon {
            return Vector3::new(-1.0, 0.0, 0.0);
        }
        if (p.x - max.x).abs() < epsilon {
            return Vector3::new(1.0, 0.0, 0.0);
        }
        if (p.y - min.y).abs() < epsilon {
            return Vector3::new(0.0, -1.0, 0.0);
        }
        if (p.y - max.y).abs() < epsilon {
            return Vector3::new(0.0, 1.0, 0.0);
        }
        if (p.z - min.z).abs() < epsilon {
            return Vector3::new(0.0, 0.0, -1.0);
        }
        Vector3::new(0.0, 0.0, 1.0)
    }

    fn intersect_cylinder(
        &self,
        _ray: &Ray,
        _transform: &Transform,
        _radius: f64,
        _height: f64,
        _object: &SceneObject,
    ) -> Option<RayHit> {
        // TODO: Implement cylinder intersection
        None
    }

    fn intersect_mesh(
        &self,
        _ray: &Ray,
        _transform: &Transform,
        _vertices: &[Position],
        _indices: &[[u32; 3]],
        _object: &SceneObject,
    ) -> Option<RayHit> {
        // TODO: Implement mesh intersection with BVH
        None
    }

    fn get_objects(&self) -> Vec<SceneObject> {
        self.objects.values().cloned().collect()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use nalgebra::{Point3, UnitQuaternion, Vector3};

    #[tokio::test]
    async fn test_native_backend_initialization() {
        let mut backend = NativeBackend::new();
        assert!(!backend.is_initialized());

        backend.initialize(BackendConfig::default()).await.unwrap();
        assert!(backend.is_initialized());
    }

    #[tokio::test]
    async fn test_scene_loading() {
        let mut backend = NativeBackend::new();
        backend.initialize(BackendConfig::default()).await.unwrap();

        let scene = backend.load_scene("test.scene").await.unwrap();
        assert_eq!(scene.backend_type, BackendType::Native);
    }

    #[tokio::test]
    async fn test_sphere_intersection() {
        let mut backend = NativeBackend::new();
        backend.initialize(BackendConfig::default()).await.unwrap();

        let scene = backend.load_scene("test.scene").await.unwrap();

        // Add a sphere at origin
        let sphere = SceneObject {
            id: "sphere1".to_string(),
            name: "Test Sphere".to_string(),
            transform: Transform::new(Point3::origin(), UnitQuaternion::identity()),
            geometry: Geometry::Sphere { radius: 1.0 },
            material: Material::concrete(),
        };

        backend.add_object(&scene, sphere).unwrap();

        // Cast ray from above
        let ray = Ray::new(Point3::new(0.0, 0.0, 5.0), Vector3::new(0.0, 0.0, -1.0));

        let hit = backend.cast_ray(&scene, &ray).unwrap();
        assert!(hit.is_some());

        let hit = hit.unwrap();
        assert_eq!(hit.object_id, "sphere1");
        assert!((hit.distance - 4.0).abs() < 1e-6);
    }
}
