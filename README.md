<br>
<p align="center" width="100%">
    <img src="./media/images/autonomysim_simple_bw_1000w.png" width="600vw" alt="AutonomySim logo">
</p>

<h1 align="center">The simulation system for robotic general intelligence&trade;</h1>

<p align="center" width="100%">
  <a alt="Windows Build"
    href="https://github.com/nervosys/AutonomySim/actions/workflows/test_windows.yml">
    <img src="https://github.com/nervosys/AutonomySim/actions/workflows/test_windows.yml/badge.svg">
  </a>
  <a alt="Linux Build"
    href="https://github.com/nervosys/AutonomySim/actions/workflows/test_linux.yml">
    <img src="https://github.com/nervosys/AutonomySim/actions/workflows/test_linux.yml/badge.svg">
  </a>
  <a alt="macOS Build"
    href="https://github.com/nervosys/AutonomySim/actions/workflows/test_macos.yml">
    <img src="https://github.com/nervosys/AutonomySim/actions/workflows/test_macos.yml/badge.svg">
  </a>
</p>

<p align="center" width="100%">
  <a alt="Source Formatting"
    href="https://github.com/nervosys/AutonomySim/actions/workflows/test_format.yml">
    <img src="https://github.com/nervosys/AutonomySim/actions/workflows/test_format.yml/badge.svg">
  </a>
  <a alt="Documentation Build"
    href="https://github.com/nervosys/AutonomySim/actions/workflows/test_docs.yml">
    <img src="https://github.com/nervosys/AutonomySim/actions/workflows/test_docs.yml/badge.svg">
  </a>
  <a alt="License"
    href="https://opensource.org/licenses/Apache-2.0">
    <img src="https://img.shields.io/badge/License-Apache%202.0-blue.svg">
  </a>
</p>

<!---
<p align="center" width="100%">
  <img src="https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white">
  <img src="https://img.shields.io/badge/Ubuntu-E95420?style=for-the-badge&logo=ubuntu&logoColor=white">
  <img src="https://img.shields.io/badge/macOS-313131?style=for-the-badge&logo=apple&logoColor=white">
</p>
--->

<p align="center" width="100%">
  <img src="https://img.shields.io/badge/Unreal-313131.svg?style=for-the-badge&logo=unrealengine&logoColor=white">
  <img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white">
  <img src="https://img.shields.io/badge/Rust-313131?style=for-the-badge&logo=rust&logoColor=white">
  <img src="https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white">
  <img src="https://img.shields.io/badge/C%23-239120?style=for-the-badge&logo=c-sharp&logoColor=white">
  <img src="https://img.shields.io/badge/BASH-313131?style=for-the-badge&logo=gnu-bash&logoColor=white">
  <img src="https://img.shields.io/badge/PWSH-2CA5E0?style=for-the-badge&logo=powershell&logoColor=white">
</p>

<p align="center" width="100vw">
  <a alt="Sponsor"
    href="https://github.com/sponsors/nervosys">
    <img src="https://img.shields.io/badge/sponsor-313131?style=for-the-badge&logo=GitHub-Sponsors&logoColor=#white">
  </a>
</p>

<p align="center">
    <img src="./docs/media/images/autonomysim_drone_manual.gif" alt="aerial vehicle" style="width:100vw">
</p>

## Announcements

### AutonomySim

- `Build systems`: updated to `C++20` standard, `CMake` 3.29.2, `Clang` 17, `GCC` 13, `Python` 3.12
- The `autonomysim` Python package has undergone a complete overhaul! `AutonomyLib` is next.
- `Windows`: we now provide separate Batch/Command and PowerShell build systems. Both are tested in CI/CD.
- `Documentation`: a new system has been rolled out that also generates Python and C++ API docs.
- Support for `Unity Engine`, `Gazebo`, and `ROS1` has been deprecated to focus on `Unreal Engine`, `ROS2`, `ArduPilot/PX4`, `qGroundControl`, `PyTorch`, and real-time applications of `AutonomyLib` via software- and hardware-in-the-loop.
- The `master` branch supports `Unreal Engine` version 5.03 and above. For version 4.27, you can use the `ue4.27` branch.

### Unreal Engine and Omniverse

- `Unreal Engine` version 5.4 brought [new features](https://www.unrealengine.com/en-US/blog/all-the-big-news-from-the-state-of-unreal-at-gdc-24) including animation and sequencing.
- `Unreal Engine` version 5.2 brought native support for Apple/ARM M-series silicon.
- `Unreal Engine` version 5.0 brought powerful new features including [Nanite](https://www.unrealengine.com/en-US/blog/understanding-nanite---unreal-engine-5-s-new-virtualized-geometry-system) and [Lumen](https://www.unrealengine.com/en-US/tech-blog/unreal-engine-5-goes-all-in-on-dynamic-global-illumination-with-lumen), while deprecating support for the [PhysX](https://developer.nvidia.com/physx-sdk) backend.
- The `Omniverse Unreal Engine Connector` enables you to sync `Unreal Engine` data with an `Omniverse Nucleus` server, which can then sync with any `Omniverse Connect` application including `IsaacSim`.

For a complete list of changes, view the [change log](./docs/CHANGELOG.md).

## Toward Robotic General Intelligence (RGI)

> "A central challenge in the branch of artificial intelligence (AI) known as machine learning (ML) is the massive amount of high-fidelity labeled data needed to train models. Datasets for real-world systems are either hand-crafted or automatically labeled using other models, introducing biases and errors into data and downstream models, and limiting learning to the offline case. While game engines have long used hardware-accelerated physics engines of Newtonian dynamics, accelerators for physics-based rendering (PBR) have recently made real-time ray-tracing a reality, extending physical realism to the visual domain. In parallel, physical fidelity with the real world has skyrocketed with the rapid growth and falling cost of Earth observation data. For the first time in history, the average user can generate high-fidelity robotic system models and real-world labeled datasets with known physics for offline or online learning of intelligent agents. This will revolutionize AI for robotics, where the data and safety requirements are otherwise intractable, while enabling low-cost hardware prototyping _in silico_." [-Dr. Adam Erickson, 2024](#)

## Introduction

`AutonomySim` is a high-fidelity, photorealistic simulator for _multi-agent and -domain autonomous systems_, _intelligent robotic systems_, or _embodiment_ as it is known in the AI research community. `AutonomySim` is based on [`Unreal Engine`](https://www.unrealengine.com/) and Microsoft's former [`AirSim`](https://github.com/microsoft/AirSim/). `AutonomySim` is an open-source, cross-platform, modular simulator for robotic intelligence that supports software-in-the-loop (SITL) and hardware-in-the-loop (HITL) operational modes for popular robotics controllers (e.g., `Pixhawk/PX4`, `APM/ArduPilot`). Future support is planned for SITL and HITL ground control software (GCS) such as `qGroundControl`. `AutonomySim` is developed as an `Unreal Engine` plugin that can be dropped into any Unreal environment or downloaded from the [Unreal Engine Marketplace](https://www.unrealengine.com/marketplace/). The aim of `AutonomySim` is to provide physically realistic multi-modal simulations of robotic systems with first-class support for popular AI and control systems libraries in order to develop new perception, actuation, communication, navigation, and coordination AI models for diverse real-world environments.

We hope that you find `AutonomySim` enjoyable to use and develop. Unlike other projects, we intend to make public any and all improvements to the software framework. We merely ask that you share your improvements in return, although you are not obligated to do so in any way. Together, we will build a foundation for robotic general intelligence (RGI) by providing the best simulation system for AI in robotics.

## Professional Services

Robotics companies interested in having [Nervosys](https://nervosys.ai/) model their hardware/software and develop related AI models in `AutonomySim` can reach us directly at opensource@nervosys.ai. We are delighted to offer our services, so that we may continue to support and improve this essential open-source robotics project for the benefit of the community.

## Supported Operating Systems

### Windows

- Windows 10
- Windows 11
- Windows Server 2019 (untested)
- Windows Server 2022 (untested)

### Linux

- Ubuntu 20.04 LTS (Focal Fossa)
- Ubuntu 22.04 LTS (Jammy Jellyfish)
- Ubuntu Server 22.04 LTS (untested)
- Ubuntu Core 22 (untested)
- Botnix 1.0 (Torbjörn) ([coming soon!](https://github.com/nervosys/Botnix/))

### macOS

> [!NOTE]
> `Unreal Engine` versions 5.2 and up natively support Apple/ARM M-series silicon.

- macOS 11 (Big Sur)
- macOS 12 (Monterey)
- macOS 13 (Ventura)
- macOS 14 (Sonoma)

## Installation

Coming soon. In the meantime, please see our GitHub Workflows for how to build the project.

## Getting Started

Below are explanations and examples to help you get started.

- [Project structure](https://nervosys.github.io/AutonomySim/project_structure.html)
- [Development workflow](https://nervosys.github.io/AutonomySim/development_workflow.html)
- [Settings](https://nervosys.github.io/AutonomySim/settings.html)
- [API examples](https://nervosys.github.io/AutonomySim/apis.html)
- [Image APIs](https://nervosys.github.io/AutonomySim/apis_image.html)
- [C++ API usage](https://nervosys.github.io/AutonomySim/apis_cpp.html)
- [Camera views](https://nervosys.github.io/AutonomySim/camera_views.html)
- [Sensors](https://nervosys.github.io/AutonomySim/sensors.html)
- [Voxel grids](https://nervosys.github.io/AutonomySim/voxel_grid.html)
- [Robot controllers](https://nervosys.github.io/AutonomySim/controller_robot.html)
- [Radio controllers](https://nervosys.github.io/AutonomySim/controller_remote.html)
- [Wired controllers](https://nervosys.github.io/AutonomySim/controller_wired.html)
- [Adding new APIs](https://nervosys.github.io/AutonomySim/apis_new.html)
- [Simple flight controller](https://nervosys.github.io/AutonomySim/simple_flight.html)
- [ROS](https://nervosys.github.io/AutonomySim/ros_pkgs.html)

## Documentation

Project documentation and autogenerated API documentation:

- [Main documentation](https://nervosys.github.io/AutonomySim/)
- [C++ API documentation](https://nervosys.github.io/AutonomySim/api/cpp/html/index.html)
- [Python API documentation](https://nervosys.github.io/AutonomySim/api/python/html/index.html)

Overview of the `AutonomySim` architecture:

<p align="center">
  <img src="./media/images/shah2018_architecture.png" width="80%" alt="architecture">
  <br>
  Figure 1. Overview of the simulation architecuture from <a href="https://arxiv.org/abs/1705.05065">Shah et al. (2017)</a>.
</p>

## Tutorials

Based on `AirSim`, the predecessor to `AutonomySim`.

### Videos

- [Setting up AirSim with Pixhawk Tutorial](https://youtu.be/1oY8Qu5maQQ) by Chris Lovett
- [Using AirSim with Pixhawk Tutorial](https://youtu.be/HNWdYrtw3f0) by Chris Lovett
- [Using off-the-self environments with AirSim](https://www.youtube.com/watch?v=y09VbdQWvQY) by Jim Piavis
- [Harnessing high-fidelity simulation for autonomous systems](https://note.microsoft.com/MSR-Webinar-AirSim-Registration-On-Demand.html) by Sai Vemprala

### Guides

- [Reinforcement Learning with AirSim](https://nervosys.github.io/AutonomySim/reinforcement_learning) by Ashish Kapoor

### Projects

- [The Autonomous Driving Cookbook](https://github.com/nervosys/AutonomySim-DrivingCookbook) by Microsoft Deep Learning and Robotics Garage Chapter
- [Using TensorFlow for simple collision avoidance](https://github.com/nervosys/AutonomySim-TensorFlow) by Simon Levy and WLU team

## Operational Modes

Mirroring real-world robotic systems, `AutonomySim` will support three different operational modes:

1. Human operation
2. Machine operation
3. Hybrid human-machine operation

### Human Operation

If you have wired or remote controller, you can manually control vehicles in the simulator as shown below. For ground vehicles, you can use the arrow keys for control inputs (i.e., steering, accelerating, decelerating). See more details [here](https://nervosys.github.io/AutonomySim/remote_control).

<p align="center">
    <img src="./docs/media/images/autonomysim_drone_manual.gif" alt="aerial vehicle" style="scale:100%">
    <img src="./docs/media/images/autonomysim_car_manual.gif" alt="ground vehicle" style="scale:100%">
</p>

### Machine Operation

`AutonomySim` exposes Application Programming Interfaces (APIs) for progammatic interaction with the simulation vehicles and environment. These APIs can be used to control vehicles and the environment (e.g., weather), generate imagery, audio, or video, record control inputs along with vehicle and environment state, _et cetera_. The APIs are exposed through a remote procedure call (RPC) interface and are accessible through a variety of languages, including C++, Python, and Rust.

The APIs are also available as part of a separate, independent, cross-platform library, so that they can be deployed on embedded systems running on your vehicle. That way, you can write and test your code in simulation, where mistakes are relatively cheap, before deploying it to real-world systems. Moreover, a core focus of `AutonomySim` is the development of simulation-to-real (sim2real) domain adaptation AI models, a form of transfer learning. These metamodels map from models of simulations to models of real-world systems, leveraging the universal function approximation abilities of artificial neural networks (ANNs) to _implicitly_ represent real-world processes not _explicitly_ represented in simulation.

> [!NOTE]
> The [Sim Mode](https://nervosys.github.io/AutonomySim/settings#simmode) setting or the new [Computer Vision](https://nervosys.github.io/AutonomySim/apis_image#computer-vision-mode-1) mode can be used to specify the default vehicle, so you don't get prompted each time you start `AutonomySim`. See [this](https://nervosys.github.io/AutonomySim/apis) for more details.

### Hybrid Human-Machine Operation

Using a form of hardware-in-the-loop (HITL), `AutonomySim` is capable of operating in hybrid human-machine mode. The classical example is a semi-autonomous aircraft stabilization program, which maps human control inputs (or lack thereof) into optimal control outputs to provide level flight.

## Generating Labeled Data for Offline Machine Learning

There are two general approaches to generating labeled data with `AutonomySim`:

1. Manual: using the `record` button
2. Programmatic: using the APIs

The first method, using the `record` button, is the easiest method. Simply press _the big red button_ in the lower right corner to begin recording. This will record the vehicle pose/state and image for each frame. The data logging code is simple and easy to customize to your application.

<p align="center">
  <img src="./docs/media/images/record_data.png" alt="record screenshot" style="scale:100%">
  <br>
  Human/manual data recording mode.
</p>

The second method, using the APIs, is a more precise and repeatable method for generating labeled data. The APIs allow you to be in full control of the _how_, _what_, _where_, and _when_ of data logging.

### Computer Vision Mode

It is possible to use `AutonomySim` with vehicles and physics disabled. This is known as `Computer Vision Mode` and it supports both human and machine control. In this mode, you can use the keyboard or APIs to position cameras in arbitrary poses and collect imagery including depth, disparity, surface normals, or object segmentation masks. As the name implies, this is useful for generating labeled data for learning computer vision models. See [this](https://nervosys.github.io/AutonomySim/apis_image) for more details.

## Labeled Data Modalities

We plan on supporting the following [sensors](https://nervosys.github.io/AutonomySim/sensors/) and data modalities:

- RGB imagery
- Depth
- Disparity
- Surface normals
- Object panoptic, semantic, and instance segmentation masks
- Object bounding boxes (coming soon)
- Audio (coming soon)
- Video (coming soon)
- Short- or long-wavelength infrared imagery ([see](https://nervosys.github.io/AutonomySim/InfraredCamera/))
- Multi- and hyper-spectral (TBD)
- LiDAR ([see](https://github.com/nervosys/AutonomySim/blob/master/docs/lidar.md); GPU acceleration coming soon)
- RaDAR (TBD)
- SoNAR (TBD)

We also plan on providing autolabeling systems in the future.

## Vehicles

### Ground

- Automobile
- BoxCar (coming soon)
- ClearPath Husky (coming soon)
- Pioneer P3DX (coming soon)

### Air

- Multirotor aircraft: Quadcopter
- Rotor-wing aircraft (TBD)
- Fixed-wing aircraft (TBD)
- Hybrid aircraft (TBD)

## Environmental Dynamics

### Weather

The weather system support human and machine control. Press the `F10` key to see the available weather effect options. You can also control the weather using the APIs, as shown [here](https://nervosys.github.io/AutonomySim/apis#weather-apis).

<p align="center">
  <img src="./docs/media/images/weather_menu.png" alt="weather menu" style="scale:100%">
  <br>
  Weather effects menu.
</p>

Press the `F1` key to see other available options.

### Procedural Terrain Generation

Unreal Engine includes built-in support.

## Machine Learning Applications

- Learning Perception, Communication, Planning, and Control Models
- Imitation or Apprenticeship Learning
  - An example of recording control inputs and vehicle state for learning control systems.
- Neural Radiance Fields
  - Learning compressed 3-D radiative transfer models.
- Large Language Models
  - An example of using a large language model (LLM) to parse text commands into planning and control inputs for robotic systems. See [Eureka](https://eureka-research.github.io/).
- Robotics Foundation Models
- Learning Surrogate Models or Emulators
- Learning World Models

## Other Applications

- Sensor System Development
- Locomotion System Development
  - An example of learning structure, actuator, and locomotion models. This is useful, for example, for developing robotic systems that are robust to major structural failures, such as the loss of motors or legs.
- Communication System Development
- Data Randomization via Procedural Modeling
  - A class of data augmentation to generate large amounts of diverse training data.

## Join the Community

For updates or answers to your questions, join our GitHub Discussion group [here](https://github.com/nervosys/AutonomySim/discussions) or our Discord channel [here](https://discord.gg/Mu97ucWa). 

For information on becoming a contributor, see the below section.

## Contributing

Community contributions are *strongly encouraged* via GitHub Issues and Pull Requests. If you are looking for areas to contribute, please take a look at the [open issues](https://github.com/nervosys/AutonomySim/issues). For more information about contributing to the project, please visit the [contributing page](https://github.com/nervosys/AutonomySim/blob/master/CONTRIBUTING.md).

Our [GitHub Insights](https://github.com/nervosys/AutonomySim/pulse/) page provides a sense of the project activity.

### Project Structure

The `AutonomySim` repository consists of multiple projects with a project, the core of which is `AutonomyLib`. Additional projects include `DroneServer`, `DroneShell`, `HelloCar`, `HelloDrone`, `MavLinkCom`, `Examples`, and `LogViewer`.

It provides wrappers for `Unreal Engine`, `Python`, and `ROS2`, as well as build scripts for `Docker` and `Azure`.

The build system uses `Visual Studio 2022` for Windows and `CMake` for cross-platform support. Pre-build scripts are run beforehand to prepare the target project for compilation.

For more information, see the following pages:

* [Project Design](https://nervosys.github.io/AutonomySim/design)
* [Code Structure](https://nervosys.github.io/AutonomySim/code_structure)
* [Contribution Guidelines](CONTRIBUTING.md)

### Comparison with Related Projects

Below is a comparison with `AirSim` and its other forks.

| Project                                                     | Origin   | Year | New Features      | Updated | Framework     | Server        | SaaS                                                          | Organization                     |
| ----------------------------------------------------------- | -------- | ---- | ----------------- | ------- | ------------- | ------------- | ------------------------------------------------------------- | -------------------------------- |
| [AirSim](https://github.com/microsoft/AirSim/)              | original | 2017 | -                 | 2022    | open-source   | closed-source | [Project AirSim](https://www.microsoft.com/en-us/ai/)         | [Microsoft](#)                   |
| [Cosys-AirSim](https://github.com/Cosys-Lab/Cosys-AirSim/)  | fork     | 2020 | Sensors, Matlab   | 2024    | open-source   | -             | -                                                             | [Cosys Lab](#)                   |
| [Colosseum](https://github.com/CodexLabsLLC/Colosseum/)     | fork     | 2022 | Unreal Engine 5   | 2023    | open-source   | closed-source | [SWARM](https://www.swarmsim.io/)                             | [Codex Labs](#)                  |
| [AirGen](#)                                                 | fork     | 2023 | -                 | -       | closed-source | closed-source | [GRID](https://github.com/ScaledFoundations/GRID-playground/) | [Scaled Foundations](#)          |
| [AirSim-Client](https://github.com/Sollimann/airsim-client) | original | 2022 | Rust              | 2023    | open-source   | -             | -                                                             | [Kristoffer Solberg Rakstad](#)  |
| [AutonomySim](#)                                            | fork     | 2023 | Major refactoring | 2024    | open-source   | open-source   | -                                                             | [Nervosys](https://nervosys.ai/) |

Compared to other simulation engines for robotic systems, `AutonomySim` is open-source and built on top of a state-of-the-art game engine with the best available features and performance. It also has batteries-included support for popular machine learning workflows.

`AutonomySim` has been designed from the ground-up for robotic general intelligence (RGI) or general robotic intelligence (GRI) based on multi-modal, high-dimensional sensing combined with state-of-the-art AI modeling techniques, terms and concepts that Nervosys rightfully [invented](https://blog.nervosys.ai/interview-at-icra/).

## Current and Past Users

A subset of the organizations, people, and projects that have used `AutonomySim` or its predecessor, `AirSim`, are listed [here](https://nervosys.github.io/AutonomySim/who_is_using).

If you would like to be featured on this list, please submit a request [here](https://github.com/nervosys/AutonomySim/issues).

## Sponsors

* [Nervosys](https://nervosys.ai/): *"Accelerating the development of robotic general intelligence"*

`AutonomySim` is made possible by [Nervosys](https://nervosys.ai/), NVIDIA, Epic Games, Microsoft, the Linux Foundation and countless contributors. We need your support to ensure the success of `AutonomySim`.

Reach out to us at opensource@nervosys.ai to learn how you can support this project.

## Roadmap

- [x] Focus on Unreal Engine, deprecate support for ROS1, Unity, Gazebo
- [x] Project reorganization and modernization
  - [x] Add support for the latest `Unreal Engine` version 5.4
  - [x] Updated Python library
  - [ ] Update C++ library
  - [ ] Add API, RPC support for Rust, deprecate support for Java and C#
  - [x] Update automated tests
- [ ] Add the JSBSim flight dynamics model (FDM) plugin for Unreal Engine per [Project Antoinette](https://www.unrealengine.com/en-US/blog/antoinette-project-tools-to-create-the-next-generation-of-flight-simulators/)
- [ ] Add libraries and tools for artificial intelligence (AI)
  - [ ] CUDA Toolkit, CuDNN, TensorRT, JetPack
  - [ ] Mojo, PyTorch, JAX-Flax, OpenCV
  - [ ] LLMs: LLaMA 3, Mistral/Mixtral, OpenHermes, SD, LLaVA
  - [ ] Robotics foundation models (multimodal)
  - [ ] Interpretability, explainability, and hard bounds or guardrails
  - [ ] Testing, safety, cybersecurity tools
- [ ] Add headless server mode for control via external program, container, virtual machine, or local network
  - [ ] Add support for SITL and HITL of companion computers (NVIDIA JetPack)
- [ ] Create generic interface for control software
  - [ ] Add flight control software (FCS): BetaFlight, OpenPilot, LibrePilot, dRehmFlight, Flightmare/flightlib
  - [ ] Add MavLink-based ground control software (GCS): qGroundControl, Mission Planner, Auterion Mission Control
  - [ ] Add self-driving car (i.e., rover) software: openpilot, Autoware, CARLA, Vista, Aslan, OpenPodcar/ROS
- [ ] Add large labeled robotics labeled datasets

## References

For technical aspects on the design of `AutonomySim`, refer to the original `AirSim` manuscript:

```bibtex
@techreport{shah2017,
  author = {Shital Shah and Debadeepta Dey and Chris Lovett and Ashish Kapoor},
  year = 2017,
  title = {{Aerial Informatics and Robotics Platform}},
  number = {MSR-TR-2017-9},
  institution = {Microsoft Research},
  url = {https://www.microsoft.com/en-us/research/project/aerial-informatics-robotics-platform/},
  eprint = {https://www.microsoft.com/en-us/research/wp-content/uploads/2017/02/aerial-informatics-robotics.pdf},
  note = {AirSim draft manuscript}
}
```

A list of manuscripts related to the design and implementation of `AutonomySim` and its predecessors can be found [here](./manuscripts.bib). Please open a GitHub Issue to add your manuscript.

A manuscript on the design and implementation of `AutonomySim` is forthcoming.

## Frequently Asked Questions (FAQ)

For other questions, see the [FAQ](https://nervosys.github.io/AutonomySim/faq) and feel free to post issues in the repository [here](https://github.com/nervosys/AutonomySim/issues).

## Code of Conduct

The AutonomySim [Code of Conduct](CODE_OF_CONDUCT.md) is based on the _Contributor Covenant version 2.1_, itself inspired by the Mozilla standards. The original unmodified covenant can be found [here](https://www.contributor-covenant.org/version/2/1/code_of_conduct.html). The changes made better reflect the core value of our organization in preserving freedom.

For answers to common questions about this code of conduct, see the [FAQ](https://www.contributor-covenant.org/faq). Translations are available [here](https://www.contributor-covenant.org/translations).

Contact us through [GitHub Discussions](https://github.com/nervosys/AutonomySim/discussions) with any additional questions or comments, so that we may maintain transparency in adopting community guidelines.

## License

This project is released under the [Apache 2.0 License](LICENSE), a permissible license often preferred for commercial use.

Any and all sublicenses can be found [here](sublicenses/).

<p align="center">
  <br>
  <img src="./media/images/logo_block_raven_rgi_modern_blueye_xwerx_bw_1000h.png" width="400vw" alt="xwerx logo">
  <br>
  <i>"Accelerating the development of robotic general intelligence"</i>
  <br>
  TM 2024 &copy; <a href="https://nervosys.ai/">Nervosys, LLC</a>
</p>
