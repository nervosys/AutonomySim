# PX4 Lockstep Mode

The latest version of PX4 supports a new [lockstep feature](https://docs.px4.io/master/en/simulation/#lockstep-simulation) when communicating with the simulator over TCP. Lockstep is an important feature because it synchronizes PX4 and the simulator so that they use the same clock time. This makes PX4 behave normally even during unusually long delays in simulator response due to performance lags.

!!! info "Recommendation"
    When you are running a lockstep enabled version of PX4 in software-in-the-loop (SITL) mode, set `AutonomySim` to use a `SteppableClock` and set `UseTcp` and `LockStep` to `true`.

```json
{
    "SettingsVersion": 1.2,
    "SimMode": "Multirotor",
    "ClockType": "SteppableClock",
    "Vehicles": {
        "PX4": {
            "VehicleType": "PX4Multirotor",
            "UseTcp": true,
            "LockStep": true,
            ...
```

This causes `AutonomySim` to not use a real-time clock, but instead to advance the clock in-step with each sensor update sent to PX4. This way, PX4 perceives time is progressing smoothly no matter how long it takes `AutonomySim` to process the update loop.

This has the following advantages:

* `AutonomySim` can be run on slow machines that cannot process updates quickly.
* You can debug `AutonomySim`, hit a breakpoint, and when you resume, PX4 will behave normally.
* You can enable very slow sensors such as LiDAR with a large number of simulated points and PX4 will still behave normally.

However, the `lockstep` feature does have side effects. These include slower update loops caused by running `AutonomySim` on an underpowered machine or by expensive sensors (e.g., LiDAR), which can create visible jerkiness in the simulated flight, if you view the on-screen updates in real-time.

# Disabling Lockstep Mode

If you are running PX4 in `Cygwin`, there is an [open issue with lockstep](https://github.com/nervosys/AutonomySim/issues/3415). PX4 is configured to use lockstep by default. To disable this feature, first [disable it in PX4](https://docs.px4.io/master/en/simulation/#disable-lockstep-simulation):

1. Navigate to `boards/px4/sitl/` in your local PX4 repository
2. Edit `default.cmake` and find the following line:
    ```cmake
    set(ENABLE_LOCKSTEP_SCHEDULER yes)
    ```
3. Change this line to:
    ```cmake
    set(ENABLE_LOCKSTEP_SCHEDULER no)
    ```
4. Disable it in AutonomySim by setting `LockStep` to `false` and either removing any `"ClockType": 
"SteppableClock"` setting or resetting `ClockType` back to default:
```json
    {
        ...
        "ClockType": "",
        "Vehicles": {
            "PX4": {
                "VehicleType": "PX4Multirotor",
                "LockStep": false,
                ...
```
5. Now you can run PX4 SITL as you normally would (`make px4_sitl_default none_iris`) and it will use the host system time without waiting on `AutonomySim`.
