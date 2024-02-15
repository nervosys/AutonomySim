from autonomysim.clients import MultirotorClient
from autonomysim.types import LandedState
from autonomysim.utils import SetupPath


def main():
    SetupPath()

    client = MultirotorClient()
    client.confirmConnection()
    client.enableApiControl(True)
    client.armDisarm(True)

    landed = client.getMultirotorState().landed_state
    if landed == LandedState.Landed:
        print("already landed...")
    else:
        print("landing...")
        client.landAsync().join()

    client.armDisarm(False)
    client.enableApiControl(False)


if __name__ == "__main__":
    main()
