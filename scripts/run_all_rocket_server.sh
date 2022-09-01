#!/bin/sh

(trap 'kill 0' SIGINT; /home/debian/rocket_server/scripts/rs422_rocket_server.sh & /home/debian/rocket_server/scripts/ignition_rocket_server.sh)

echo "\n\n\nProgram is closed, rebooting device...\n\n\n"

sudo reboot 