# Wired Controllers

To use an `XBox` or similar with `AutonomySim`, follow the below steps:

!!! note
    Xbox controllers can be emulated by theoretically any controller in software. A number of programs exist for this purpose, such as [HideHide](https://github.com/nefarius/HidHide), [X360CE](https://github.com/x360ce/x360ce), or [DS4Windows](https://ds4-windows.com/). In fact, you can also write Python scripts to emulate physical controllers if so desired.

1. Connect the XBox controller so that it shows up in your PC Game Controllers:

![game controllers](media/images/game_controllers.png)

2. Launch `QGroundControl` and you should see a new Joystick tab under settings:

![game controllers](media/images/qgc_joystick.png)

Now calibrate the radio, and setup some handy button actions. For example, I set mine so that 
the `A` button arms the drone, `B` put it in manual flight mode, `X` puts it in altitude hold mode and `Y` puts it in position hold mode. I also prefer the feel of the controller when I check the box labelled `Use exponential curve on roll, pitch, yaw` because this gives me more sensitivity for small movements.

`QGroundControl` will find your Pixhawk via the UDP proxy port 14550 setup by `MavLinkTest` above. `AutonomySim` will find your Pixhawk via the other UDP server port 14570 also setup by MavLinkTest above. You can also use all the QGroundControl controls for autonomous flying at this point too.

3. Connect to Pixhawk serial port using MavLinkTest.exe like this:

```shell
MavLinkTest.exe -serial:*,115200 -proxy:127.0.0.1:14550 -server:127.0.0.1:14570
```

1. Run `AutonomySim` with the following settings in `AutonomySim/settings.json`:

```json
"Vehicles": {
    "PX4": {
        "VehicleType": "PX4Multirotor",

        "SitlIp": "",
        "SitlPort": 14560,
        "UdpIp": "127.0.0.1",
        "UdpPort": 14570,
        "UseSerial": false
    }
}
```

## Advanced

If the Joystick tab doesn't show up in QGroundControl then Click on the purple "Q" icon on left in tool bar to reveal the Preferences panel. Go to General tab and check the Virtual Joystick checkbox.  Go back to settings screen (gears icon), click on Parameters tab,
type `COM_RC_IN_MODE` in search box and change its value to either `Joystick/No RC Checks` or `Virtual RC by Joystick`.

### Other Options

See [remote controller options](controllers_remote.md)
