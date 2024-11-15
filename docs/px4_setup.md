# PX4 Setup

The [PX4 software stack](http://github.com/px4/firmware) is an open-source popular flight controller with support for wide variety of boards and sensors as well as built-in capability for higher level tasks such as mission planning. Please visit [px4.io](http://px4.io) for more information.

!!! warning "Setting up PX4 can be challenging"
    While all releases of `AutonomySim` are always tested with PX4 to ensure support, setting up PX4 is not a trivial task. Unless you have at least intermediate level of experience with PX4 stack, we recommend you use [simple_flight](simple_flight.md), which is now a default in `AutonomySim`.

## Supported Hardware

The following Pixhawk hardware versions have been tested and verified to work with `AutonomySim`:

* [Pixhawk PX4 2.4.8](http://www.banggood.com/Pixhawk-PX4-2_4_8-Flight-Controller-32-Bit-ARM-PX4FMU-PX4IO-Combo-for-Multicopters-p-1040416.html)
* [PixFalcon](https://hobbyking.com/en_us/pixfalcon-micro-px4-autopilot.html?___store=en_us)
* [PixRacer](https://www.banggood.com/Pixracer-Autopilot-Xracer-V1_0-Flight-Controller-Mini-PX4-Built-in-Wifi-For-FPV-Racing-RC-Multirotor-p-1056428.html?utm_source=google&utm_medium=cpc_ods&utm_content=starr&utm_campaign=Smlrfpv-ds-FPVracer&gclid=CjwKEAjw9MrIBRCr2LPek5-h8U0SJAD3jfhtbEfqhX4Lu94kPe88Zrr62A5qVgx-wRDBuUulGzHELRoCRVTw_wcB)
* [Pixhawk 2.1](http://www.proficnc.com/)
* [Pixhawk 4 mini from Holybro](https://shop.holybro.com/pixhawk4-mini_p1120.html)
* [Pixhawk 4 from Holybro](https://shop.holybro.com/pixhawk-4beta-launch_p1089.html)

PX4 firmware version 1.11.2 also works on the Pixhawk 4 devices.

## PX4 Hardware-in-the-loop (HITL) Setup

First, you will need one of the supported device listed above. For manual flight, you will also need an RC transmitter and receiver.

1. Ensure your RC receiver is bound with its RC transmitter. Connect the RC transmitter to the flight controller's RC port. Refer to your RC manual and [PX4 docs](https://docs.px4.io/en/getting_started/rc_transmitter_receiver.html) for more information.
2. Download [QGroundControl](http://qgroundcontrol.com/), launch it and connect your flight controller to the USB port.
3. Use `QGroundControl` to flash the latest PX4 Flight Stack.
See also [initial firmware setup video](https://docs.px4.io/master/en/config/).
4. In `QGroundControl`, configure your Pixhawk for HITL simulation by selecting the HITL Quadrocopter X airframe. After PX4 reboots, check that `HIL Quadrocopter X` is selected.
5. In `QGroundControl`, go to `Radio` tab and calibrate (make sure the remote control is on and the receiver is showing the indicator for the binding).
6. Go to the `Flight Mode` tab and chose one of the remote control switches as `Mode Channel`. Then set (for example) `Stabilized` and `Attitude` flight modes for two positions of the switch.
7. Go to the `Tuning` section of QGroundControl and set appropriate values. For example, for the Turnigy/FrSky/FlySky FS-TH9X RC transmitter, the following values give a more realistic feel:
   * Hover Throttle = mid+1 mark
   * Roll and pitch sensitivity = mid-3 mark
   * Altitude and position control sensitivity = mid-2 mark
8. In [AutonomySim settings](settings.md) file, specify PX4 for your vehicle config like this:

```json
    {
        "SettingsVersion": 1.2,
        "SimMode": "Multirotor",
        "ClockType": "SteppableClock",
        "Vehicles": {
            "PX4": {
                "VehicleType": "PX4Multirotor",
                "UseSerial": true,
                "LockStep": true,
                "Sensors":{
                    "Barometer":{
                        "SensorType": 1,
                        "Enabled": true,
                        "PressureFactorSigma": 0.0001825
                    }
                },
                "Parameters": {
                    "NAV_RCL_ACT": 0,
                    "NAV_DLL_ACT": 0,
                    "COM_OBL_ACT": 1,
                    "LPE_LAT": 47.641468,
                    "LPE_LON": -122.140165
                }
            }
        }
    }
```

!!! note
    The PX4 `[simulator]` is communicating over TCP, so we need to set `"UseTcp": true,`. We have also enabled `LockStep` (see [PX4 LockStep](px4_lockstep.md) for more information). The `Barometer` setting keeps PX4 stable because the default `AutonomySim` barometer generates excessive noise. This setting improves the signal-to-noise ratio (SNR), producing faster GPS lock on PX4.

With the above settings, you should be able to use a radio/remote controller (RC) in `AutonomySim`. You can usually arm the vehicle by bringing two RC sticks or 'pots' downward and inward toward one another. You do not need `QGroundControl` after the initial setup. Typically, the `Stabilized` rather than `Manual` mode provides new users or 'newbs' a better experience. See [PX4 Basic Flying Guide](https://docs.px4.io/master/en/flying/basic_flying.html).

You can also control the drone from [Python APIs](apis.md).

See [Walkthrough Demo Video](https://youtu.be/HNWdYrtw3f0) and  [Unreal AutonomySim Setup  Video](https://youtu.be/1oY8Qu5maQQ) that shows you all the setup steps in this document.

## Setting up PX4 Software-in-Loop

The PX4 SITL mode doesn't require you to have separate device such as a Pixhawk or Pixracer. This is in fact the recommended way to use PX4 with simulators by PX4 team. However, this is indeed harder to set up. Please see [this dedicated page](px4_sitl.md) for setting up PX4 in SITL mode.

## FAQ

### Drone doesn't fly properly, it just goes "crazy".

There are a few reasons that can cause this. First, make sure your drone doesn't fall down large distance when starting the simulator. This might happen if you have created a custom Unreal environment and Player Start is placed too high above the ground. It seems that when this happens internal calibration in PX4 gets confused.

You should [also use QGroundControl](#setting-up-px4-hardware-in-loop) and make sure you can arm and takeoff in QGroundControl properly.

Finally, this also can be a machine performance issue in some rare cases, check your [hard drive performance](hard_drive.md).

### Can I use Arducopter or other MavLink implementations?

Our code is tested with the [PX4 firmware](https://dev.px4.io/). We have not tested Arducopter or other mavlink implementations. Some of the flight API's do use the PX4 custom modes in the MAV_CMD_DO_SET_MODE messages (like PX4_CUSTOM_MAIN_MODE_AUTO)

### It is not finding my Pixhawk hardware

Check your settings.json file for this line "SerialPort":"*,115200".  The asterisk here means "find any serial port that looks like a Pixhawk device, but this doesn't always work for all types of Pixhawk hardware. So on Windows you can find the actual COM port using Device Manager, look under "Ports (COM & LPT), plug the device in and see what new COM port shows up.  Let's say you see a new port named "USB Serial Port (COM5)". Well, then change the SerialPort setting to this: "SerialPort":"COM5,115200".

On Linux, the device can be found by running "ls /dev/serial/by-id" if you see a device name listed that looks like this `usb-3D_Robotics_PX4_FMU_v2.x_0-if00` then you can use that name to connect, like this: `"SerialPort":"/dev/serial/by-id/usb-3D_Robotics_PX4_FMU_v2.x_0-if00"`. Note that this long name is actually a symbolic link to the real name, if you use `"ls -l ..."` you can find that symbolic link, it is usually something like `"/dev/ttyACM0"`, so this will also work `"SerialPort":"/dev/ttyACM0,115200"`.  But that mapping is similar to windows, it is automatically assigned and can change, whereas the long name will work even if the actual TTY serial device mapping changes.

### WARN [commander] Takeoff denied, disarm and re-try

This happens if you try and take off when PX4 still has not computed the home position. PX4 will report the home position once it is happy with the GPS signal, and you will see these messages:

```shell
INFO  [commander] home: 47.6414680, -122.1401672, 119.99
INFO  [tone_alarm] home_set
```

Up until this point in time, however, the PX4 will reject takeoff commands.

### When I tell the drone to do something it always lands

For example, you use DroneShell `moveToPosition -z -20 -x 50 -y 0` which it does, but when it gets to the target location the drone starts to land. This is the default behavior of PX4 when offboard mode completes. To set the drone to hover instead set this PX4 parameter:

```shell
param set COM_OBL_ACT 1
```

### I get message length mismatches errors

You might need to set MAV_PROTO_VER parameter in QGC to "Always use version 1". Please see [this issue](https://github.com/nervosys/AutonomySim/issues/546) more details.
