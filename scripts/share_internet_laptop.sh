#!/bin/sh

sudo iptables -t nat -F
sudo iptables -t mangle -F
sudo iptables -F
sudo iptables -X

#echo "Type the id of the WAN"
#read wan_name
#sudo iptables --table nat --append POSTROUTING --out-interface $wan_name -j MASQUERADE

sudo iptables --table nat --append POSTROUTING --out-interface wlp12s0 -j MASQUERADE


#echo "Type BBB USB intarface id"
#read bbb_name
#sudo iptables --append FORWARD --in-interface $bbb_name -j ACCEPT

sudo iptables --append FORWARD --in-interface enx38d269393db0  -j ACCEPT
