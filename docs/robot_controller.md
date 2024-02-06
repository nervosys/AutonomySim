# Robot Controller

![UVX Micronav](images\micronav_gcs.png "UVX Micronav")

## What is a Robot Controller?

A robot controller is any device or computer program used to control a robot. Robot controllers generally take one of five forms:

1. **Human wetware controller**
   - Handheld remote control (RC) or radio frequency (RF) controller
     - ACSST/FASST/DSM2/ELRS/Crossfire/Tracer, ZigBee, LoRaWAN, WiFi HaLow, or 5G/6G handheld controller
     - Protocol-agnostic software-defined radio (SDR) handheld controller
   - Handheld hard-wired controller
     - Serial/USB game-pad, flight-stick, or keyboard controller
     - Ethernet pendant controller (common for industrial robotic arms, where the pendant also performs computation)
2. **Computer software controller**
   - A software architecture or control system
   - ArduPilot, PX4, BetaPilot, OpenPilot, dRehmFlight, etc.
3. **Computer hardware controller**
   - Runs a computer software control system
   - ArduPilot Mega (APM) 2.6, Pixhawk V6X, VOXL 2, SpeedyBee F405, Blade F722, GOKU GN745, RUSH Core 7, Naze32, DJI Naza, etc.
4. **Mechanical hardware controller**
   - The earliest autopilot systems were purely mechanical
   - Many assembly lines or continuous-flow production machines still use mechanical controllers
5. **Hybrid controller**
   - Most controllers today are human-assisted computer controllers
   - Examples include the DJI RC Pro Remote Controller and Skydio Controller, which are handheld computers, as well as traditional fly-by-wire and stabilization modes
   - Human-in-the-loop (HITL) or human-computer interaction (HCI) for semi-autonomous systems
   - Robots are fundamentally cyberphysical systems
6. **Remote Controller**
   - Any controller operated _ex situ_ rather than _in situ_

While (1) involves manual control, (2-4) involve automatic control, (5) involves a semi-automatic control, and (6) involves a variety of control approaches.

Other types of controllers, such as electrochemical controllers (as in the nervous system) and chemosensory controllers (as in pheromone signaling), remain theoretical. We do, however, use magnetic fields for navigation and thus control, similar to [a wide variety of animal species](https://doi.org/10.1007%2Fs00359-021-01507-0).

## What is a Flight Controller?

A flight controller is simply a robot controller for the airborne or atmospheric domain. Colloquially, the term 'flight controller' commonly refers to a flight control unit (FCU) or computer, such as a microcontroller, running flight control software on a real-time operating system (RTOS). The flight control hardware and software together comprise a flight control system (FCS). Alternatively, the term 'flight controller' may also refer to a handheld RC transmitter, used to control aircraft for over a century (Tesla demonstrated radio control in 1898, following the 1870 wired demonstration by Siemens).

## Which Other Domains Exist for Robot Controllers?

There are robot controllers for every domain on Earth and beyond:

- Air
- Ground
- Surface (water)
- Subsurface (water)
- Space
- Other Worlds

These domains can be found in common robotic system classifications:

- Unmanned Aerial Vehicle (UAV)
- Unmanned Ground Vehicle (UGV)
- Unmanned Surface Vehicle (USV)
- Unmanned Underwater Vehicle (UUV)
- Unmanned Spacecraft (US)

## Why Do I Need a Robot Controller for a Simulator?

Robots are empty vassels without a sense of agency or goal-directed behaviour. Breaking these _plans_ down into a series of _actions_ further necessitates a _control system_ to translate the human and/or computer inputs into desired outputs or behaviour. The primary job of the _control system_ is to estimate the _current state_ from sensor data, determine the desired _future state_ and required _control inputs_ to optimally converge toward this state, and translate _control inputs_ into _actuator and robot motion_.

```mermaid
%%{init: {"graph": {"htmlLabels": false}}}%%
graph LR;

   S_previous["` $State_{t-1}$ `"];

   A[State Previous (t-1)];
   B[State Physical (t0)];
   C[Sensor Noise];
   D[Sensors Current (t0)];
   E[State Estimator];
   F[State Current (t0)];
   G[Path Planner];
   H[State Future (t+1)];
   I[Actuator Commands];
   J[Actuator Noise];
   K[Actuator Future];

   State Previous (t_-1);
   State Physical + Encoder Noise --> Sensors Current;
   State Previous + Sensors Current + State Estimator --> State Current (t_0);
   State Current + Path Planner --> State Desired (t_+1);
   State Current + Actuator Commands --> State Future;

    A --> B;
    B --> C;
    C --> D;
    D --> E;
```

Where sensors noise is related to resolution, precision, latency, hysteresis, thermal, optical, electromagnetic, and model error.

`$S_{t-1}$`

The state estimator is commonly a form of [Kálmán filter](https://arxiv.org/abs/1910.03558/) (not to be confused with his contemporary campatriot, [von Kármán](https://en.wikipedia.org/wiki/Theodore_von_K%C3%A1rm%C3%A1n)):

```latex
\mathrm{Initial State}
\mathbf{x}_0 P_0

\mathrm{Prediction}
\mathbf{x}_{k+1}^{(P)} &= A \mathbf{x}_k + B {\color{orange} a_k} P_{k+1}^{(P)} &= A P_k A^\tran + C_k^{(r_s)}

\mathrm{Update}
k \leftarrow k + 1

\mathrm{Innovation}
K_k &= P_k^{(P)} H^\tran {\left (H P_k^{(P)} H^\tran + C_k^{(r_m)} \right)}^{-1} {\color{blue} \mathbf{x}_k} &= (I - K_k H) \mathbf{x}_k^{(P)} + K_k {\color{orange} z_k}{\color{blue} P_k} &= (I - K_k H) P_k^{(P)}
```

For quadrotors, desired state can be specified as roll, pitch and yaw, for example. It then estimates actual roll, pitch and yaw using gyroscope and accelerometer. Then it generates appropriate motor signals so actual state becomes desired state. You can find more in-depth in [our paper](paper/main.pdf).

## How Do Robot Controllers Communicate with the Simulator?

Simulator consumes the motor signals generated by flight controller to figure out force and thrust generated by each actuator (i.e. propellers in case of quadrotor). This is then used by the physics engine to compute the kinetic properties of the vehicle. This in turn generates simulated sensor data and feed it back to the flight controller. You can find more in-depth in [our paper](paper/main.pdf).

## What is Hardware-in-the-loop and Software-in-the-loop?

Hardware-in-the-loop (HITL) means that the flight controller runs on physical hardware, such as an APM, Pixhawk, or Naze32 flight control unit (FCU). While some FCUs use real-time (`RT_PREEMPT`) Linux operating systems (OSes), typically running on a 64-bit ARM system-on-chip (SoC), most FCUs use hard real-time OSes running on microcontrollers.

You then connect this hardware to PC using USB port. Simulator talks to the device to retrieve actuator signals and send it simulated sensor data. This is obviously as close as you can get to real thing. However, it typically requires more steps to set up and usually hard to debug. One big issue is that simulator clock and device clock runs on their own speed and accuracy. Also, USB connection (which is usually only USB 2.0) may not be enough for real-time communication.

Software-in-the-loop (SITL) means that the flight controller software runs on your computer as opposed to separate board. This is generally fine except that now you are not touching any code paths that are specific to your device. Also, none of your code now runs with real-time clock usually provided by specialized hardware board. For well-designed flight controllers with software clock, these are usually not concerning issues.

## Which Flight Controllers are Supported?

AutonomySim has built-in flight controller called [simple_flight](simple_flight.md) and it is used by default. You don't need to do anything to use or configure it. AutonomySim also supports [PX4](px4_setup.md) & [ArduPilot](https://ardupilot.org/dev/docs/sitl-with-AutonomySim.html) as external flight controllers for advanced users.

## Using AutonomySim Without Flight Controller

Yes, now it's possible to use AutonomySim without flight controller. Please see the [instructions here](image_apis.md) for how to use so-called "Computer Vision" mode. If you don't need vehicle dynamics, we highly recommend using this mode.
