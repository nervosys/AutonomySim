# Change Log

## 2024-04-23

* `Build systems`: updated to C++20 standard, CMake 3.29.2, Clang 17, GCC 13, Python 3.12
* Added support for the latest [Unreal Engine 5.4](https://www.unrealengine.com/en-US/blog/all-the-big-news-from-the-state-of-unreal-at-gdc-24)!

## 2024-01-25

* The `autonomysim` Python package has undergone a complete overhaul! `AutonomyLib` is next.
* `Windows`: We now provide separate Batch/Command and PowerShell build systems. Both are tested in CI/CD.
* `Documentation`: A new system has been rolled out that also generates Python and C++ API docs.
* Support for `Unity Engine`, `Gazebo`, and `ROS1` has been deprecated to focus on `Unreal Engine`, `ROS2`, `ArduPilot/PX4`, `qGroundControl`, `PyTorch`, and real-time applications of `AutonomyLib` via software- and hardware-in-the-loop.
* `Linux`: added `ROS2` support for `Ubuntu 22.04 LTS` (Jammy Jellyfish).

## Antiquity

* `Unreal Engine` version 5.0 brought powerful new features including [Nanite](https://www.unrealengine.com/en-US/blog/understanding-nanite---unreal-engine-5-s-new-virtualized-geometry-system) and [Lumen](https://www.unrealengine.com/en-US/tech-blog/unreal-engine-5-goes-all-in-on-dynamic-global-illumination-with-lumen), while deprecating support for the [PhysX](https://developer.nvidia.com/physx-sdk) backend.
* `macOS`: `Unreal Engine` version 5.2 brought native support for Apple/ARM M-series silicon.
* The `master` branch supports `Unreal Engine` version 5.03 and above. For version 4.27, you can use the `ue4.27` branch.
* The `Omniverse Unreal Engine Connector` makes it possible to sync `Unreal Engine` data with an `Omniverse Nucleus` server, which can then sync with any `Omniverse Connect` application including `IsaacSim`.
