# Orbital Trajectories

Moved here from [https://github.com/nervosys/AutonomySim/wiki/An-Orbit-Trajectory](https://github.com/nervosys/AutonomySim/wiki/An-Orbit-Trajectory)

Have you ever wanted to fly a nice smooth circular orbit? This can be handy for capturing 3-D objects from all sides, especially if you get multiple orbits at different altitudes.

The `PythonClient/multirotor` folder contains a script named [Orbit](https://github.com/nervosys/AutonomySim/blob/master/PythonClient/multirotor/orbit.py) that will do precisely that.

See [demo video](https://youtu.be/RFG5CTQi3Us)

The demo video was created by running this command line:

```shell
python orbit.py --radius 10 --altitude 5 --speed 1 --center "0,1" --iterations 1
```

This flies a 10-meter radius orbit around the center location at `(startpos + radius * [0,1])`. In other words, the center is located `radius` meters away in the direction of the provided center vector. This also keeps the front-facing camera on the drone always pointing at the center of the circle. If you watch the flight using `LogViewer`, you will see a nice circular pattern get traced out on the GPS map:

![image](media/images/orbit.png)

The core of the algorithm is uncomplicated. At each point on the circle, we look ahead by a small delta in degrees, called the `lookahead_angle`, with the angle computed based on our desired velocity. We then find the lookahead point on the circle using sine/cosine and make that our target point. Calculating the velocity is then easy; just subtract the current position from that point and feed this into the AutonomySim method, `moveByVelocityZ()`.
