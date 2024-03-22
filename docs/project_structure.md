# Project Structure

## AutonomyLib

The majority of the code is located in `AutonomyLib`, a self-contained library that can be compiled with any popular `C++11` compiler.

`AutonomyLib` consists of the following components:

1. [*Physics engine:*](https://github.com/nervosys/AutonomySim/tree/master/AutonomyLib/include/physics) This is header-only physics engine. It is designed to be fast and extensible to implement different vehicles.
2. [*Sensor models:*](https://github.com/nervosys/AutonomySim/tree/master/AutonomyLib/include/sensors) These are header-only models for the barometer, IMU, GPS and magnetometer.
3. [*Vehicle models:*](https://github.com/nervosys/AutonomySim/tree/master/AutonomyLib/include/vehiclesr) These are header-only models for vehicle configurations and models. Currently, we have implemented model for a multirotor and a configuration for a PX4 quadrotor in the `X config`. There are several different multirotor models defined in `MultirotorParams.hpp`, including a hexacopter as well.
4. [*API-related files:*](https://github.com/nervosys/AutonomySim/tree/master/AutonomyLib/include/api) This part of `AutonomyLib` provides abstract base classes for our APIs and concrete implementations for specific vehicle platforms such as `MavLink`. It also contains classes for the RPC client and server.

Apart from these, all common utilities are defined in the [`common/`](https://github.com/nervosys/AutonomySim/tree/master/AutonomyLib/include/common) subfolder. One important file here is [`AutonomySimSettings.hpp`](https://github.com/nervosys/AutonomySim/blob/master/AutonomyLib/include/common/AutonomySimSettings.hpp), which should be modified if any new fields are to be added in `settings.json`.

`AutonomySim` supports different firmwares for multirotors such as its own `SimpleFlight`, `PX4`, and `ArduPilot`. Files for communicating with each firmware are placed in their respective subfolders in [`multirotor/firmwares`](https://github.com/nervosys/AutonomySim/tree/master/AutonomyLib/include/vehicles/multirotor/firmwares).

Vehicle-specific APIs are defined in the `api/` subfolder along-with required data structures. The [`AutonomyLib/src/`](https://github.com/nervosys/AutonomySim/tree/master/AutonomyLib/src) directory contains `.cpp` files with implementations of various methods defined in the `.hpp` files. For e.g. [`MultirotorApiBase.cpp`](https://github.com/nervosys/AutonomySim/blob/master/AutonomyLib/src/vehicles/multirotor/api/MultirotorApiBase.cpp) contains the base implementation of the multirotor APIs, which can be overridden in the specific firmware files.

## Unreal/Plugins/AutonomySim

This is the only portion of project which is dependent on Unreal engine. We have kept it isolated so we can implement simulator for other platforms as well. The Unreal code takes advantage of its UObject based classes including Blueprints. The `Source/` folder contains the C++ files, while the `Content/` folder has the blueprints and assets. Some main components are described below:

1. *SimMode_ classes*: The SimMode classes help implement many different modes, such as pure Computer Vision mode, where there is no vehicle or simulation for a specific vehicle (currently car and multirotor). The vehicle classes are located in [`Vehicles/`](https://github.com/nervosys/AutonomySim/tree/master/Unreal/Plugins/AutonomySim/Source/Vehicles)
2. *PawnSimApi*: This is the [base class](https://github.com/nervosys/AutonomySim/blob/master/Unreal/Plugins/AutonomySim/Source/PawnSimApi.cpp) for all vehicle pawn visualizations. Each vehicle has their own child (Multirotor|Car|ComputerVision)Pawn class.
3. [UnrealSensors](https://github.com/nervosys/AutonomySim/tree/master/Unreal/Plugins/AutonomySim/Source/UnrealSensors): Contains implementation of Distance and Lidar sensors.
4. *WorldSimApi*: Implements most of the environment and vehicle-agnostic APIs

Apart from these, [`PIPCamera`](https://github.com/nervosys/AutonomySim/blob/master/Unreal/Plugins/AutonomySim/Source/PIPCamera.cpp) contains the camera initialization, and [`UnrealImageCapture`](https://github.com/nervosys/AutonomySim/blob/master/Unreal/Plugins/AutonomySim/Source/UnrealImageCapture.cpp) & [`RenderRequest`](https://github.com/nervosys/AutonomySim/blob/master/Unreal/Plugins/AutonomySim/Source/RenderRequest.cpp) the image rendering code. [`AutonomyBlueprintLib`](https://github.com/nervosys/AutonomySim/blob/master/Unreal/Plugins/AutonomySim/Source/AutonomyBlueprintLib.cpp) has a lot of utility and wrapper methods used to interface with the UE4 engine.

## MavLinkCom

This is the library developed by our own team member [Chris Lovett](https://github.com/lovettchris) that provides C++ classes to talk to the MavLink devices. This library is stand alone and can be used in any project. See [MavLinkCom](mavlink_comm.md) for more info.

## Sample Programs

We have created a few sample programs to demonstrate how to use the API. See HelloDrone and DroneShell. DroneShell demonstrates how to connect to the simulator using UDP.  The simulator is running a server (similar to DroneServer).

## Python Client

[PythonClient](https://github.com/nervosys/AutonomySim/tree/master/PythonClient) contains Python API wrapper files and sample programs demonstrating their uses.

## Unreal Engine

The below figure illustrates how `AutonomySim` is loaded and invoked by the Unreal Engine:

![AutonomySimConstruction](media/images/autonomysim_startup.png)

## Contributing

See [Contribution Guidelines](CONTRIBUTING.md)
