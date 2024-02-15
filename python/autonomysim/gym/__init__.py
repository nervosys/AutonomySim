from gymnasium.envs.registration import register

description = """
AutonomyGym
"""

register(
    id="autonomysim-drone-example-v0",
    entry_point="autonomygym.envs:AutonomySimDroneEnv",
)

register(
    id="autonomysim-car-example-v0",
    entry_point="autonomygym.envs:AutonomySimCarEnv",
)
