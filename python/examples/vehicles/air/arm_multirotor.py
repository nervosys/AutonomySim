from autonomysim.clients import MultirotorClient
from autonomysim.utils import SetupPath


def main():
    SetupPath()

    client = MultirotorClient()
    client.confirmConnection()
    client.armDisarm(True)


if __name__ == "__main__":
    main()
