import setup_path
import AutonomySim

import pprint

def print_state():
    print("===============================================================")
    state = client.getMultirotorState()
    print("state: %s" % pprint.pformat(state))
    return state


client = AutonomySim.MultirotorClient()
state = print_state()

if state.ready:
    print("drone is ready!")
else:
    print("drone is not ready!")
if state.ready_message:
    print(state.ready_message)
