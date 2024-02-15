import pprint

from autonomysim.clients import MultirotorClient
from autonomysim.utils import SetupPath


def get_state(client):
    print("===============================================================")
    state = client.getMultirotorState()
    print("state: %s" % pprint.pformat(state))
    return state


def main():
    SetupPath()

    client = MultirotorClient()
    state = get_state(client)

    if state.ready:
        print("drone is ready!")
    else:
        print("drone is not ready!")
    if state.ready_message:
        print(state.ready_message)


if __name__ == "__main__":
    main()
