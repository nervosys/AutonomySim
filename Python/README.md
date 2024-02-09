# AutonomySim Python API

This package contains Python APIs for [AutonomySim](https://github.com/nervosys/AutonomySim).

## How to Use

See examples at [car/hello_car.py](https://github.com/nervosys/AutonomySim/blob/master/PythonClient/car/hello_car.py) or [multirotor/hello_drone.py](https://github.com/nervosys/AutonomySim/blob/master/PythonClient/multirotor/hello_drone.py). You can import `AutonomSim` as follows:

```python
import autonomysim as sim
```

## Dependencies

This package depends on `msgpack` and would automatically install `msgpack-rpc-python` (this may need administrator/sudo prompt):

```
pip install msgpack-rpc-python
```

Some examples also requires opencv.

## More Information

More information on the `AutonomySim` Python APIs can be found [here](https://github.com/nervosys/AutonomySim/blob/master/docs/python.md).
