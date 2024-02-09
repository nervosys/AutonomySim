#!/bin/bash
#
# Docker entrypoint for AutonomySim on Azure
#

AUTONOMYSIM_EXECUTABLE='/home/autonomysim_user/Blocks/Blocks.sh'

echo 'Starting AutonomySim binary...'
${AUTONOMYSIM_EXECUTABLE} &

echo 'Waiting 10 seconds before starting app...'
sleep 10

echo 'Starting Python app'
python3.11 /home/autonomysim_user/app/multirotor.py