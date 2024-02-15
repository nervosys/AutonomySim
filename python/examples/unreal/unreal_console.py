from autonomysim.unreal.commands import RunCmdList
from autonomysim.clients import MultirotorClient


def main():
    client = MultirotorClient()
    client.confirmConnection()
    RunCmdList(client)


if __name__ == "__main__":
    main()
