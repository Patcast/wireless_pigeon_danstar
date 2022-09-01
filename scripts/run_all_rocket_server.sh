#!/bin/sh

(trap 'kill 0' SIGINT; rs422_rocket_server.sh & ignition_rocket_server.sh)