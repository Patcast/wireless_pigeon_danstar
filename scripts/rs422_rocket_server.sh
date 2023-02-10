#!/bin/sh

stty -F /dev/ttyO1 sane
stty speed 921600 -F /dev/ttyO1
stty -F /dev/ttyO1

socat -d -d -d file:/dev/ttyO1,ispeed=921600,ospeed=921600,raw,echo=0 tcp-l:3222,reuseaddr,fork 