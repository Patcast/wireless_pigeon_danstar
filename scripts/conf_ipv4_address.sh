#!/bin/sh

echo "Type the id of the LAN"
read lan_name
sudo ip addr flush dev $lan_name

echo "Type the new ipV4 address for the LAN"
read lan_addr
sudo ip addr add $lan_addr /27 dev $lan_name
echo "Address successfully changed "
