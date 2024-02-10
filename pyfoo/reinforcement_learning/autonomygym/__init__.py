from gym.envs.registration import register

register(
    id="autonomysim-drone-sample-v0",
    entry_point="autonomygym.envs:AutonomySimDroneEnv",
)

register(
    id="autonomysim-car-sample-v0",
    entry_point="autonomygym.envs:AutonomySimCarEnv",
)
