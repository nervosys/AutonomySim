# Remote Control

To fly drones manually, a physical (or software-emulated) controller is required. Alternatively, you may (a) wrap [application programming interfaces (APIs)](apis.md) calls for software control or (b) use the [computer vision mode](image_apis.md) for manual keyboard control.

## Default Configuration

By default, AutonomySim uses the [simple flight controller](simple_flight.md), which connects your computer to a physical controller via USB port.

## Controller Notes

To date, XBox and [FrSky Taranis X9D Plus](https://hobbyking.com/en_us/frsky-2-4ghz-accst-taranis-x9d-plus-and-x8r-combo-digital-telemetry-radio-system-mode-2.html) controllers have been verified as supported. Other controllers from Microsoft, Sony, Logitech, FrSky/Turnigy, Spektrum, Futaba, and TBS may also work. If you have a remote control (RC) or radio-frequency (RF) controller that lacks USB support, you will need to convert the signal to USB. Such converters are often called trainer cables or dongles. Learn how to make your own [here](https://github.com/patolin/rc-receiver-joystick). If you have a [Steam Deck](https://www.steamdeck.com/), which runs a flavor of Arch Linux (SteamOS), you may be able to run AutonomySim directly on your controller.

AutonomySim can detect large variety of devices. However, devices other than those listed above may require extra configuration. In the future, we may add relared configuration options in the `settings.json` file. If your controller does not work, we recommend trying workarounds such as [x360ce](http://www.x360ce.com/) or modifying the [SimJoystick.cpp file](https://github.com/nervosys/AutonomySim/blob/main/Unreal/Plugins/AutonomySim/Source/SimJoyStick/SimJoyStick.cpp#L50).

NOTE: If a realistic experience is desired, the XBox 360 controller is not recommended as it has insufficient potentiometer encoding precision. For more information, see the FAQ below.

### FrSky Taranis X9D Plus

The [FrSky Taranis X9D Plus](https://hobbyking.com/en_us/frsky-2-4ghz-accst-taranis-x9d-plus-and-x8r-combo-digital-telemetry-radio-system-mode-2.html) is a modern RC controller with a USB port so that it can directly connect to PCs. [Download the AutonomySim config file](misc/AutonomySim_FrSkyTaranis.bin) and [follow this tutorial](https://www.youtube.com/watch?v=qe-13Gyb0sw) to import it into your RC. You should then see `sim` model in the RC controller with all channels properly configured. As this controller runs the open-source OpenTX transmitter software, it may be adapted to other controllers as well.

### Linux

The current default configuation on Linux uses an Xbox controller. Other controllers may not properly function. In the future, we may add the ability to configure RC controllers in the `settings.json` file. For now, you may have to modify the [SimJoystick.cpp file](https://github.com/nervosys/AutonomySim/blob/main/Unreal/Plugins/AutonomySim/Source/SimJoyStick/SimJoyStick.cpp#L340) to support other devices.

## Controller Configuration for PX4

AutonomySim supports the [PX4](https://px4.io/) flight control system (FCS), typically run on a Pixhawk flight control unit (FCU). However, PX4 requires additional setup. There are many remote control options available for multi-rotor aircraft. We have successfully used the FrSky Taranis X9D Plus, FlySky FS-TH9X, and Futaba 14SG RC controllers with AutonomySim. The steps to configure your RC controller are as follows:

1. If you are going to use hardware-in-the-loop (HITL) mode, as opposed to software-in-the-loop (SITL) mode, you need a compatible receiver to bind to your RC transmitter. For more information, see the manual for your RC controller.
2. For hardware-in-the-loop (HITL) mode, connect the transmitter directly to Pixhawk. View the online documentation and/or YouTube tutorials on how to do so.
3. [Calibrate your controller in QGroundControl](https://docs.qgroundcontrol.com/en/SetupView/Radio.html).

Please see the [PX4 controller configuration](https://docs.px4.io/en/getting_started/rc_transmitter_receiver.html) and [this guide](https://docs.px4.io/master/en/getting_started/rc_transmitter_receiver.html#px4-compatible-receivers) for more information. 

### XBox 360 Controller

You can also use an xbox controller in SITL mode, it just won't be as precise as a real RC controller.
See [xbox controller](xbox_controller.md) for details on how to set that up.

### Playstation 3 Controller

A Playstation 3 controller is confirmed to work as an AutonomySim controller. On Windows, an emulator to make it look like an Xbox 360 controller, is required however. Many different solutions are available online, for example [x360ce Xbox 360 Controller Emulator](https://github.com/x360ce/x360ce).

### DJI Controller

Nils Tijtgat wrote an excellent blog on how to get the [DJI controller working with AutonomySim](https://timebutt.github.io/static/using-a-phantom-dji-controller-in-AutonomySim/).

## FAQ

1. **AutonomySim says my USB controller is not detected.**
   
   This typically happens if you have multiple RCs and or XBox/Playstation gamepads etc connected. In Windows, hit Windows+S key and search for "Set up USB Game controllers" (in older versions of Windows try "joystick"). This will show you all game controllers connected to your PC. If you don't see yours than Windows haven't detected it and so you need to first solve that issue. If you do see yours but not at the top of the list (i.e. index 0) than you need to tell AutonomySim because AutonomySim by default tries to use RC at index 0. To do this, navigate to your `~/Documents/AutonomySim` folder, open up `settings.json` and add/modify following setting. Below tells AutonomySim to use RC at `index = 2`.

    ```json
    {
        "SettingsVersion": 1.2,
        "SimMode": "Multirotor",
        "Vehicles": {
            "SimpleFlight": {
                "VehicleType": "SimpleFlight",
                "RC": {
                  "RemoteControlID": 2
                }
            }
        }
    }
    ```

2. **The vehicle is unstable when using XBox/PS3 controller**

    Regular gamepads are not very precise and have lot of random noise. Most of the times you may see significant offsets as well (i.e. output is not zero when sticks are at zero). So this behavior is expected.

3. **Where is the RC controller calibration utility in AutonomySim?**

    We haven't implemented it yet. This means your RC firmware will need to have a capability to do calibration for now.

4. **The RC controller is not working with PX4**

    First, ensure your RC controller is working in [QGroundControl](https://docs.qgroundcontrol.com/en/SetupView/Radio.html). If it doesn't then it will sure not work in AutonomySim. The PX4 mode is suitable for folks who have at least intermediate level of experience to deal with various issues related to PX4 and we would generally refer you to get help from PX4 forums.
