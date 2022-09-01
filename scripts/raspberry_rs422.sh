#!/bin/sh

sudo socat -d -d -d file:/dev/ttyAMA0,b921600,raw,echo=0 TCP:192.168.192.218:3222,reuseaddr
