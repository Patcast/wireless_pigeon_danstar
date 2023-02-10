#!/bin/sh

X=1
while[$x -le 5]
do 
    sudo socat -d -d -d file:/dev/ttyAMA0,b921600,raw,echo=0 TCP:192.168.192.218:3222,reuseaddr
    sleep 3
done

echo "\n\n\socat is clossing\n\n\n"

 