
(trap 'kill 0' SIGINT; /home/debian/ignition_box/scripts/rs422_ignition_box.sh & /home/debian/ignition_box/scripts/ignition_box.sh)




echo "\n\n\nProgram is closed, rebooting device...\n\n\n"

sudo reboot 

# chmod +x <fileName> to run it as an executable