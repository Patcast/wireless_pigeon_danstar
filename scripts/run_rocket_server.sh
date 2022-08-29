#!/bin/sh


/home/debian/rocket_server/build/rocket_server  85354 /home/debian/rocket_server/keys/server.crt /home/debian/rocket_server/keys/server_rsa_private.pem.unsecure /home/debian/rocket_server/keys/ca.crt

echo "\n\n\nProgram is closed, rebooting device...\n\n\n"

sudo reboot 

# chmod +x <fileName> to run it as an executable