from autonomysim.clients import MultirotorClient
from autonomysim.utils import SetupPath


def main():
    SetupPath()

    # connect to the AutonomySim simulator
    client = MultirotorClient()
    client.confirmConnection()
    client.enableApiControl(True)

    home = client.getHomeGeoPoint()
    print("home:\n%s" % home)

    target = home
    target.latitude -= 1

    result = client.simTestLineOfSightToPoint(target)
    print("test line of sight from vehicle to\n%s\n\t:%s" % (target, result))

    result = client.simTestLineOfSightBetweenPoints(home, target)
    print("test line of sight from home to\n%s\n\t:%s" % (target, result))

    result = client.simGetWorldExtents()
    print("world extents:\n%s\n\t-\n%s" % (result[0], result[1]))

    client.reset()
    client.armDisarm(False)

    # that's enough fun for now. let's quit cleanly
    client.enableApiControl(False)


if __name__ == "__main__":
    main()
