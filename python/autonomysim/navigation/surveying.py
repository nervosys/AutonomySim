import sys
import time

from autonomysim.types import DrivetrainType, YawMode, LandedState, Vector3r
from autonomysim.clients import MultirotorClient


class SurveyNavigator:
    def __init__(self, args):
        self.boxsize = args.size
        self.stripewidth = args.stripewidth
        self.altitude = args.altitude
        self.velocity = args.speed
        self.client = MultirotorClient()
        self.client.confirmConnection()
        self.client.enableApiControl(True)

    def start(self):
        print("arming the drone...")
        self.client.armDisarm(True)

        landed = self.client.getMultirotorState().landed_state
        if landed == LandedState.Landed:
            print("taking off...")
            self.client.takeoffAsync().join()

        landed = self.client.getMultirotorState().landed_state
        if landed == LandedState.Landed:
            print("takeoff failed - check Unreal message log for details")
            return

        # AutonomySim uses NED coordinates so negative axis is up.
        x = -self.boxsize
        z = -self.altitude

        print("climbing to altitude: " + str(self.altitude))
        self.client.moveToPositionAsync(0, 0, z, self.velocity).join()

        print("flying to first corner of survey box")
        self.client.moveToPositionAsync(
            x, -self.boxsize, z, self.velocity).join()

        # let it settle there a bit.
        self.client.hoverAsync().join()
        time.sleep(2)

        # after hovering we need to re-enabled api control for next leg of the
        # trip
        self.client.enableApiControl(True)

        # now compute the survey path required to fill the box
        path = []
        distance = 0
        while x < self.boxsize:
            distance += self.boxsize
            path.append(Vector3r(x, self.boxsize, z))
            x += self.stripewidth
            distance += self.stripewidth
            path.append(Vector3r(x, self.boxsize, z))
            distance += self.boxsize
            path.append(Vector3r(x, -self.boxsize, z))
            x += self.stripewidth
            distance += self.stripewidth
            path.append(Vector3r(x, -self.boxsize, z))
            distance += self.boxsize

        print("starting survey, estimated distance is " + str(distance))
        trip_time = distance / self.velocity
        print("estimated survey time is " + str(trip_time))
        try:
            result = self.client.moveOnPathAsync(
                path,
                self.velocity,
                trip_time,
                DrivetrainType.ForwardOnly,
                YawMode(False, 0),
                self.velocity + (self.velocity / 2),
                1,
            ).join()
        except Exception:
            errorType, value, traceback = sys.exc_info()
            print("moveOnPath threw exception: " + str(value))
            pass

        print("flying back home")
        self.client.moveToPositionAsync(0, 0, z, self.velocity).join()

        if z < -5:
            print("descending")
            self.client.moveToPositionAsync(0, 0, -5, 2).join()

        print("landing...")
        self.client.landAsync().join()

        print("disarming.")
        self.client.armDisarm(False)
