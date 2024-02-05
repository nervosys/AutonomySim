from gym.envs.registration import register

register(
    id="AutonomySim-drone-sample-v0", entry_point="autonomygym.envs:AutonomySimDroneEnv",
)

register(
    id="AutonomySim-car-sample-v0", entry_point="autonomygym.envs:AutonomySimCarEnv",
)
