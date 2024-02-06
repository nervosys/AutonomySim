# Code Structure

## AutonomyLib

Majority of the code is located in AutonomyLib. This is a self-contained library that you should be able to compile with any C++11 compiler.

AutonomyLib consists of the following components:

1. [*Physics engine:*](https://github.com/nervosys/AutonomySim/tree/main/AutonomyLib/include/physics) This is header-only physics engine. It is designed to be fast and extensible to implement different vehicles.
2. [*Sensor models:*](https://github.com/nervosys/AutonomySim/tree/main/AutonomyLib/include/sensors) This is header-only models for Barometer, IMU, GPS and Magnetometer.
3. [*Vehicle models:*](https://github.com/nervosys/AutonomySim/tree/main/AutonomyLib/include/vehiclesr) This is header-only models for vehicle configurations and models. Currently we have implemented model for a Multirotor and a configuration for PX4 QuadRotor in the X config. There are several different Multirotor models defined in MultirotorParams.hpp including a hexacopter as well.
4. [*API-related files:*](https://github.com/nervosys/AutonomySim/tree/main/AutonomyLib/include/api) This part of AutonomyLib provides abstract base class for our APIs and concrete implementation for specific vehicle platforms such as MavLink. It also has classes for the RPC client and server.

Apart from these, all common utilities are defined in [`common/`](https://github.com/nervosys/AutonomySim/tree/main/AutonomyLib/include/common) subfolder. One important file here is [AutonomySimSettings.hpp](https://github.com/nervosys/AutonomySim/blob/main/AutonomyLib/include/common/AutonomySimSettings.hpp) which should be modified if any new fields are to be added in `settings.json`.

AutonomySim supports different firmwares for Multirotor such as its own SimpleFlight, PX4 and ArduPilot, files for communicating with each firmware are placed in their respective subfolders in [`multirotor/firmwares`](https://github.com/nervosys/AutonomySim/tree/main/AutonomyLib/include/vehicles/multirotor/firmwares).

The vehicle-specific APIs are defined in the `api/` subfolder, along-with required structs. The [`AutonomyLib/src/`](https://github.com/nervosys/AutonomySim/tree/main/AutonomyLib/src) contains .cpp files with implementations of various mehtods defined in the .hpp files. For e.g. [MultirotorApiBase.cpp](https://github.com/nervosys/AutonomySim/blob/main/AutonomyLib/src/vehicles/multirotor/api/MultirotorApiBase.cpp) contains the base implementation of the multirotor APIs, which can also be overridden in the specific firmware files if required.

## Unreal/Plugins/AutonomySim

This is the only portion of project which is dependent on Unreal engine. We have kept it isolated so we can implement simulator for other platforms as well, as has been done for [Unity](https://microsoft.github.io/AutonomySim/Unity.html). The Unreal code takes advantage of its UObject based classes including Blueprints. The `Source/` folder contains the C++ files, while the `Content/` folder has the blueprints and assets. Some main components are described below:

1. *SimMode_ classes*: The SimMode classes help implement many different modes, such as pure Computer Vision mode, where there is no vehicle or simulation for a specific vehicle (currently car and multirotor). The vehicle classes are located in [`Vehicles/`](https://github.com/nervosys/AutonomySim/tree/main/Unreal/Plugins/AutonomySim/Source/Vehicles)
2. *PawnSimApi*: This is the [base class](https://github.com/nervosys/AutonomySim/blob/main/Unreal/Plugins/AutonomySim/Source/PawnSimApi.cpp) for all vehicle pawn visualizations. Each vehicle has their own child (Multirotor|Car|ComputerVision)Pawn class.
3. [UnrealSensors](https://github.com/nervosys/AutonomySim/tree/main/Unreal/Plugins/AutonomySim/Source/UnrealSensors): Contains implementation of Distance and Lidar sensors.
4. *WorldSimApi*: Implements most of the environment and vehicle-agnostic APIs

Apart from these, [`PIPCamera`](https://github.com/nervosys/AutonomySim/blob/main/Unreal/Plugins/AutonomySim/Source/PIPCamera.cpp) contains the camera initialization, and [`UnrealImageCapture`](https://github.com/nervosys/AutonomySim/blob/main/Unreal/Plugins/AutonomySim/Source/UnrealImageCapture.cpp) & [`RenderRequest`](https://github.com/nervosys/AutonomySim/blob/main/Unreal/Plugins/AutonomySim/Source/RenderRequest.cpp) the image rendering code. [`AutonomyBlueprintLib`](https://github.com/nervosys/AutonomySim/blob/main/Unreal/Plugins/AutonomySim/Source/AutonomyBlueprintLib.cpp) has a lot of utility and wrapper methods used to interface with the UE4 engine.

## MavLinkCom

This is the library developed by our own team member [Chris Lovett](https://github.com/lovettchris) that provides C++ classes to talk to the MavLink devices. This library is stand alone and can be used in any project.
See [MavLinkCom](mavlinkcom.md) for more info.

## Sample Programs

We have created a few sample programs to demonstrate how to use the API. See HelloDrone and DroneShell.
DroneShell demonstrates how to connect to the simulator using UDP.  The simulator is running a server (similar to DroneServer).

## PythonClient

[PythonClient](https://github.com/nervosys/AutonomySim/tree/main/PythonClient) contains Python API wrapper files and sample programs demonstrating their uses.

## Unreal Framework

The following picture illustrates how AutonomySim is loaded and invoked by the Unreal Game Engine:

![AutonomySimConstruction](images/AutonomySim_startup.png)

## Contributing

See [Contribution Guidelines](CONTRIBUTING.md)
