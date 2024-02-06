# Upgrading API Client Code
There have been several API changes in AutonomySim v1.2 that we hope removes inconsistency, adds future extensibility and presents cleaner interface. Many of these changes are however *breaking changes* which means you will need to modify your client code that talks to AutonomySim.

## Quicker Way

While most changes you need to do in your client code are fairly easy, a quicker way is simply to take a look at the example code such as [Hello Drone](https://github.com/nervosys/AutonomySim/tree/main/PythonClient//multirotor/hello_drone.py)or [Hello Car](https://github.com/nervosys/AutonomySim/tree/main/PythonClient//car/hello_car.py) to get gist of changes.

## Importing AutonomySim

Instead of this:

```python
from AutonomySimClient import *
```
use this:

```python
import AutonomySim
```

Above assumes you have installed AutonomySim module using, 

```shell
pip install --user AutonomySim
```

If you are running you code from PythonClient folder in repo then you can also do this:

```python
import setup_path 
import AutonomySim
```

Here setup_path.py should exist in your folder and it will set the path of `AutonomySim` package in `PythonClient` repo folder. All examples in PythonClient folder uses this method.

## Using AutonomySim Classes

As we have everything now in package, you will need to use explicit namespace for AutonomySim classes like shown below.

Instead of this:

```python
client1 = CarClient()
```

use this:

```python
client1 = AutonomySim.CarClient()
```

## AutonomySim Types

We have moved all types in `AutonomySim` namespace.

Instead of this:

```python
image_type = AutonomySimImageType.DepthVis

d = DrivetrainType.MaxDegreeOfFreedom
```

use this:

```python
image_type = AutonomySim.ImageType.DepthVis

d = AutonomySim.DrivetrainType.MaxDegreeOfFreedom
```

## Getting Images

Nothing new below, it's just combination of above. Note that all APIs that previously took `camera_id`, now takes `camera_name` instead. You can take a look at [available cameras](image_apis.md#avilable_cameras) here.

Instead of this:

```python
responses = client.simGetImages([ImageRequest(0, AutonomySimImageType.DepthVis)])
```

use this:

```python
responses = client.simGetImages([AutonomySim.ImageRequest("0", AutonomySim.ImageType.DepthVis)])
```

## Utility Methods

In earlier version, we provided several utility methods as part of `AutonomySimClientBase`. These methods are now moved to `AutonomySim` namespace for more pythonic interface.

Instead of this:

```python
AutonomySimClientBase.write_png(my_path, img_rgba) 

AutonomySimClientBase.wait_key('Press any key')
```

use this:

```python
AutonomySim.write_png(my_path, img_rgba)

AutonomySim.wait_key('Press any key')
```

## Camera Names

AutonomySim now uses [names](image_apis.md#available_cameras) to reference cameras instead of index numbers. However to retain backward compatibility, these names are aliased with old index numbers as string.

Instead of this:

```python
client.simGetCameraInfo(0)
```

use this:

```python
client.simGetCameraInfo("0")

# or

client.simGetCameraInfo("front-center")
```

## Async Methods

For multirotors, AutonomySim had various methods such as `takeoff` or `moveByVelocityZ` that would take long time to complete. All of such methods are now renamed by adding the suffix *Async* as shown below.

Instead of this:

```python
client.takeoff()

client.moveToPosition(-10, 10, -10, 5)
```

use this:

```python
client.takeoffAsync().join()

client.moveToPositionAsync(-10, 10, -10, 5).join()
```

Here `.join()` is a call on Python's `Future` class to wait for the async call to complete. You can also choose to do some other computation instead while the call is in progress.

## Simulation-Only Methods

Now we have clear distinction between methods that are only available in simulation from the ones that may be available on actual vehicle. The simulation only methods are prefixed with `sim` as shown below.

```python
getCollisionInfo()      is renamed to       simGetCollisionInfo()
getCameraInfo()         is renamed to       simGetCameraInfo()
setCameraOrientation()  is renamed to       simSetCameraOrientation()
```

## State Information

Previously `CarState` mixed simulation-only information like `kinematics_true`. Moving forward, `CarState` will only contain information that can be obtained in real world.

```python
k = car_state.kinematics_true
```

use this:

```python
k = car_state.kinematics_estimated

# or

k = client.simGetGroundTruthKinematics()
```
