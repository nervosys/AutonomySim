#!/bin/bash
#
# Docker entrypoint for AutonomySim on Azure
#

AutonomySim_EXECUTABLE='/home/AutonomySim_user/Blocks/Blocks.sh'

echo 'Starting AutonomySim binary...'
${AutonomySim_EXECUTABLE} &

echo 'Waiting 10 seconds before starting app...'
sleep 10

echo 'Starting Python app'
python3.11 /home/AutonomySim_user/app/multirotor.py