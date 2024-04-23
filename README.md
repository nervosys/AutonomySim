<br>
<p align="center" width="100%">
    <img src="./media/images/autonomysim_simple_bw_1000w.png" width="600vw" alt="AutonomySim logo">
</p>

<h1 align="center">The simulation engine for autonomous systems</h1>

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
  <img src="https://img.shields.io/badge/PWSH-2CA5E0?style=for-the-badge&logo=powershell&logoColor=white">
  <img src="https://img.shields.io/badge/BASH-313131?style=for-the-badge&logo=gnu-bash&logoColor=white">
  <img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white">
  <img src="https://img.shields.io/badge/Rust-313131?style=for-the-badge&logo=rust&logoColor=white">
  <img src="https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white">
  <img src="https://img.shields.io/badge/C%23-239120?style=for-the-badge&logo=c-sharp&logoColor=white">
</p>

<p align="center" width="100%">
  <a alt="Sponsor"
    href="https://github.com/sponsors/nervosys">
    <img src="https://img.shields.io/badge/sponsor-313131?style=for-the-badge&logo=GitHub-Sponsors&logoColor=#white">
  </a>
</p>

## Announcements

* We are currently adding support for [Unreal Engine 5.4](https://www.unrealengine.com/en-US/blog/all-the-big-news-from-the-state-of-unreal-at-gdc-24)!
* The `autonomysim` Python package has undergone a complete overhaul! `AutonomyLib` is next.
* `Windows`: We now provide separate Batch/Command and PowerShell build systems. Both are tested in CI/CD.
* A new documentation system has been rolled out that covers the Python and C++ APIs.
* `Unreal Engine` version 5.0 brought powerful new features including [Nanite](https://www.unrealengine.com/en-US/blog/understanding-nanite---unreal-engine-5-s-new-virtualized-geometry-system) and [Lumen](https://www.unrealengine.com/en-US/tech-blog/unreal-engine-5-goes-all-in-on-dynamic-global-illumination-with-lumen), while deprecating support for the [PhysX](https://developer.nvidia.com/physx-sdk) backend.
* `macOS`: `Unreal Engine` version 5.2 brought native support for Apple/ARM M-series silicon.
* The `master` branch supports `Unreal Engine` version 5.03 and above. For version 4.27, you can use the `ue4.27` branch.
* Support for `Unity Engine`, `Gazebo`, and `ROS1` has been deprecated to focus on `Unreal Engine`, `ROS2`, `ArduPilot/PX4`, `qGroundControl`, `PyTorch`, and real-time applications of `AutonomyLib` via software- and hardware-in-the-loop.
* `Linux`: added `ROS2` support for `Ubuntu 22.04 LTS` (Jammy Jellyfish).
* The `Omniverse Unreal Engine Connector` makes it possible to sync `Unreal Engine` data with an `Omniverse Nucleus` server, which can then sync with any `Omniverse Connect` application including `IsaacSim`.

For a complete list of changes, view the [change log](./docs/CHANGELOG.md).

## Vision

> "A central challenge in machine learning (ML), a branch of artificial intelligence (AI), is the massive amount of high-fidelity data needed to train models. Datasets for real-world systems are either hand-crafted or automatically labeled using other models, introducing biases and errors into data and downstream models while limiting learning to the offline case. Although game engines have long used hardware-accelerated physics engines of Newtonian dynamics for motion, new accelerators for physics-based rendering (PBR) have made real-time ray-tracing a reality, extending physical realism to the visual domain. Realism only continues to improve with the rapid growth of Earth observation data. For the first time in history, the average user can generate high-fidelity labeled datasets with known physics for offline or online learning. This is revolutionizing AI for robotics, where the data and safety requirements are often otherwise intractable. We invite you to join us in our quest to develop physical AI by contributing to AutonomySim." [-Dr. Adam Erickson, 2024](#)

## Introduction

`AutonomySim` is a high-fidelity, photorealistic simulator for *multi-agent and -domain autonomous systems*, *intelligent robotic systems*, or *embodiment* as it is known in the AI community. `AutonomySim` is built on [`Unreal Engine`](https://www.unrealengine.com/) and based on Microsoft [`AirSim`](https://github.com/microsoft/AirSim/). It is an open-source, cross-platform, modular simulator for AI in robotics that supports software-in-the-loop (SITL) and hardware-in-the-loop (HITL) operational modes for popular flight controllers (e.g., `Pixhawk/PX4`, `APM/ArduPilot`). Future support is planned for ground control software (GCS) including `qGroundControl`. `AutonomySim` is developed as an `Unreal Engine` plugin that can be dropped into any Unreal environment or downloaded from the Epic Marketplace. The goal of `AutonomySim` is to provide physically realistic multi-modal simulations with popular built-in libraries and application programming interfaces (APIs) for the development of new sensing, communication, actuation, and AI systems in physically realistic environments.

After an exhaustive analysis of existing solutions, [Nervosys](https://nervosys.ai/) created  `AutonomySim` for the development of physical AI models that can be deployed on real-world robotic systems. Like many companies, we chose to build in software first for its increased speed and reduced cost of development. We would love it if you found `AutonomySim` useful for your needs as well. Unlike `AirSim` and related projects, we intend to make public any and all improvements to the software framework. We ask that you share your improvements in return, although you are not obligated in any way to do so. Our goal is simply to provide the most advanced simulator for intelligent robotic systems.

## Professional Services

Robotics companies interested in having Nervosys model their hardware and/or software in `AutonomySim` can us directly [here](mailto:info@nervosys.ai). We would be delighted to offer our services! That way, we can continue to support and improve this project.

## Supported Operating Systems

Below is a list of officially supported operating systems. We recommend using Windows until Linux support improves.

### Windows

- Windows 10
- Windows 11
- Windows Server 2019 (untested)
- Windows Server 2022 (untested)

### Linux

- Ubuntu 20.04 LTS (Focal Fossa)
- Ubuntu 22.04 LTS (Jammy Jellyfish) - Vulkan incompatibility, Docker recommended
- Ubuntu Server 22.04 LTS (untested)
- Ubuntu Core 22 (untested)
- [Botnix 1.0](https://github.com/nervosys/Botnix/) (in development)

### macOS

> [!NOTE]
> `Unreal Engine` version 5.2 and up provide native support for Apple/ARM M-series silicon

- macOS 11 (Big Sur)
- macOS 12 (Monterey)
- macOS 13 (Ventura)
- macOS 14 (Sonoma)

## Getting Started

Coming soon.

[](https://nervosys.github.io/AutonomySim/apis/)
[](https://nervosys.github.io/AutonomySim/dev_workflow/)

## Documentation

For details on all aspects of `AutonomySim`, view the [documentation](https://nervosys.github.io/AutonomySim/).

For an overview of the simulation architecture, see the below figure.

<p align="center">
  <img src="./media/images/shah2018_architecture.png" width="65%" alt="architecture">
  <br>
  Overview of the simulation architecuture from <a href="https://arxiv.org/abs/1705.05065">Shah et al. (2017)</a>.
</p>

## Demonstrations

Coming soon.

[](https://youtube.com/)

## Operational Modes

Mirroring real-world robotic systems, `AutonomySim` will support three different operational modes:

1. Human operation
2. Machine operation
3. Hybrid human-machine operation

### Human Operation

If you have wired or remote controller, you can manually control vehicles in the simulator as shown below. For ground vehicles, you can use the arrow keys for control inputs (i.e., steering, accelerating, decelerating). See more details [here](https://nervosys.github.io/AutonomySim/remote_control).

<p align="center">
    <img src="./docs/media/images/autonomysim_drone_manual.gif" alt="aerial vehicle" style="width:100%">
    <img src="./docs/media/images/autonomysim_car_manual.gif" alt="ground vehicle" style="width:100%">
</p>

### Machine Operation

`AutonomySim` exposes Application Programming Interfaces (APIs) for progammatic interaction with the simulation vehicles and environment. These APIs can be used to control vehicles and the environment (e.g., weather), generate imagery, audio, or video, record control inputs along with vehicle and environment state, _et cetera_. The APIs are exposed through a remote procedure call (RPC) interface and are accessible through a variety of languages, including C++, Python, C#, and Java.

The APIs are also available as part of a separate, independent, cross-platform library, so that they can be deployed on a real-time embedded system on your vehicle. That way, you can write and test your code in simulation, where mistakes are relatively cheap, before deploying it to real-world systems. Moreover, a core focal area of `AutonomySim` is the development of simulation-to-real (sim2real) domain adaptation AI models, a form of transfer learning. These metamodels map from models of simulations to models of real-world systems, leveraging the universal function approximation abilities of artificial neural networks to _implicitly_ represent real-world processes not _explicitly_ represented in simulations.

Note that you can use [Sim Mode](https://nervosys.github.io/AutonomySim/settings#simmode) setting to specify the default vehicle or the new [Computer Vision](https://nervosys.github.io/AutonomySim/apis_image#computer-vision-mode-1) mode, so you don't get prompted each time you start `AutonomySim`. See [this](https://nervosys.github.io/AutonomySim/apis) for more details.

### Hybrid Human-Machine Operation

Using a form of hardware-in-the-loop (HITL), `AutonomySim` is capable of operating in hybrid human-machine mode. The classical example is a semi-autonomous aircraft stabilization program, which maps human control inputs (or lack thereof) into optimal control outputs.

## Generating Labeled Data for Offline Machine Learning

There are two general approaches to generating labeled data with `AutonomySim`:

1. Using the `record` button manually
2. Using the APIs programmatically

The first method, using the `record` button, is the easiest method. Simply press the big red button in the lower right corner to begin recording. This will record the vehicle pose/state and image for each frame. The data logging code is simple and easy to customize to your application.

<p align="center">
  <img src="./docs/media/images/record_data.png" alt="record screenshot" style="scale:100%">
  <br>
  Human/manual data recording mode.
</p>

The second method, using the APIs, is a more precise and repeatable method for generating labeled data. The APIs allow you to be in full control of the _how_, _what_, _where_, and _when_ of data logging.

### Computer Vision Mode

It is also possible to use `AutonomySim` with vehicles and physics disabled. This is known as Computer Vision Mode and it supports both human and machine control. In this mode, you can use the keyboard or APIs to position cameras in arbitrary poses and collect imagery including depth, disparity, surface normals, or object segmentation masks. As the name implies, this is useful for generating labeled data for learning computer vision models. See [this](https://nervosys.github.io/AutonomySim/apis_image) for more details.

## Labeled Data Modalities

The following [sensors](https://nervosys.github.io/AutonomySim/sensors/) and data modalities are either available or planned:

* RGB imagery
* Depth
* Disparity
* Surface normals
* Object panoptic, semantic, and instance segmentation masks
* Object bounding boxes (coming soon)
* Audio (coming soon)
* Video (coming soon)
* Short- or long-wavelength infrared imagery ([see](https://nervosys.github.io/AutonomySim/InfraredCamera/))
* Multi- and Hyper-spectral (coming soon)
* LiDAR ([see](https://github.com/nervosys/AutonomySim/blob/master/docs/lidar.md); GPU acceleration coming soon)
* RaDAR (coming soon)
* SoNAR (coming soon)

Autolabeling systems may be added in the future.

## Vehicles

### Ground

* Automobile
* BoxCar (coming soon)
* ClearPath Husky (coming soon)
* Pioneer P3DX (coming soon)

### Air

* Quadcopter

## Machine Learning Applications

### Learning Perception, Communication, Planning, and Control Models

Coming soon.

### Imitation or Apprenticeship Learning

Coming soon. An example of recording control inputs and vehicle state for learning control systems.

### Neural Radiance Fields

Coming soon. Learning compressed 3-D radiative transfer models.

### Large Language Models

Coming soon. An example of using a large language model (LLM) to parse text commands into planning and control inputs for robotic systems. See [Eureka](https://eureka-research.github.io/).

### Learning Surrogate Models or Emulators

Coming soon.

### Learning World Models

Coming soon.

## Other Applications

### Sensor System Development

Coming soon.

### Locomotion System Development

Coming soon. An example of learning structure, actuator, and locomotion models. This is useful, for example, for developing robotic systems that are robust to major structural failures, such as the loss of motors or legs.

### Communication System Development

Coming soon.

### Simulating Specific or General Environments

Coming soon.

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

Coming soon.

## Tutorials

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

## Join the Community

For updates or answers to your questions, join our GitHub Discussion group [here](https://github.com/nervosys/AutonomySim/discussions) or our Discord channel [here](https://discord.gg/Mu97ucWa). 

For information on becoming a contributor, see the following section.

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

## Current and Past Users

A subset of the organizations, people, and projects that have used `AutonomySim` or its predecessor, `AirSim`, are listed [here](https://nervosys.github.io/AutonomySim/who_is_using).

If you would like to be featured on this list, please submit a request [here](https://github.com/nervosys/AutonomySim/issues).

## Roadmap

- [x] Focus on Unreal Engine, deprecate support for ROS1, Unity, Gazebo
- [x] Project reorganization and modernization (restructuring, renaming, refactoring, porting, updating)
  - [x] Add support for the latest `Unreal Engine` version 5.3
  - [ ] Add API, RPC support for Rust, deprecate support for Java and C#
  - [ ] Update automated tests
- [ ] Add libraries and tools for artificial intelligence (AI)
  - [ ] CUDA Toolkit, CuDNN, TensorRT
  - [ ] Python, Mojo, PyTorch, JAX, Flax, MLX, OpenCV
  - [ ] Generative models: LLaMA 2, Mistral/Mixtral, OpenHermes, SD, LLaVA
  - [ ] Robotics foundation models
  - [ ] Multi-modal models
  - [ ] Interpretability, explainability, and hard bounds or guardrails
  - [ ] Safety and cybersecurity
- [ ] Add headless server mode for control via external program, container, virtual machine, or local network
  - [ ] Add NVIDIA JetPack and Botnix OS support for software-in-the-loop (SITL)
- [ ] Add the JSBSim flight dynamics model (FDM) plugin for Unreal Engine per [Project Antoinette](https://www.unrealengine.com/en-US/blog/antoinette-project-tools-to-create-the-next-generation-of-flight-simulators/)
- [ ] Add flight control software (FCS): BetaFlight, OpenPilot, LibrePilot, dRehmFlight, Flightmare/flightlib
- [ ] Add MavLink-based ground control software (GCS): qGroundControl, Mission Planner, Auterion Mission Control
- [ ] Add self-driving car (i.e., rover) software: openpilot, Autoware, CARLA, Vista, Aslan, OpenPodcar/ROS
- [ ] Add NVIDIA Omniverse IsaacSim/Gym interoperability

## Sponsors

* [Nervosys](https://nervosys.ai/): *"Accelerating the development of robotic general intelligence"*

## Donations

`AutonomySim` is made possible by [Nervosys](https://nervosys.ai/), NVIDIA, Epic Games, Microsoft, the
Linux Foundation and countless contributors to related projects.

We need your support to ensure the success of `AutonomySim` development. Reach out to us at opensource@nervosys.ai to learn how you can support this project.

## Background

`AutonomySim` began as an update to the open-source `AirSim` project, which Microsoft
shutdown in July of 2022 to focus on their closed-source cloud software-as-a-service (SaaS)
version. Our first task was to update `AirSim` to support `Unreal Engine` 5, which we soon
discovered was in already in development at other organizations. Unfortunately, these
organizations only seemed to be interested in creating closed-source cloud SaaS platforms
similar to Microsoft, which had resulted in the `AirSim` project being archived. Fearing a repeat of this outcome, we wanted to take the project in a new, open, multi-agent, -domain, and -modal direction. We are not very interested in cloud platforms, which are simply other peoples' computers,
but rather on running `AutonomySim` in our own secure enclaves. It is, after all, a game engine. We want to see it in all its glory and think you will too.

While `Unreal Engine` is well-suited to simulating the terrestrial domain due to its classical Newtonian physics engine, the aerial domain is better represented by dedicated flight dynamics models (FDMs). These small models approximate much larger computational fluid dynamics (CFD) models that are too expensive to run in real-time. Thus, it makes little sense to limit `AutonomySim` to the aerial domain and individual agents, given that multi-agent, -domain, and -modal simulation capabilities are needed to operate in complex real-world systems. We hope that we, as a community, can bring the marine and aerial domains to parity with dedicated simulators.

Are you a fluid dynamics expert? We would love to have your input.

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
