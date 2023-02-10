# Pigeon Project

This was a project that was done in collaboration with Danstar (http://www.danstar.dk), the largest rocketry student asociation in DK. The project was a solution for the communication between Mission control and Launching site at about 1km of distance. The ethernet protocol was used, TLS was used for the ignition signals and standard TCP was used for all ther communication. 

## This repository contains:
- C code: Handles the encrypted communication between both sides and modifies the state of the GPIOS of the server and client Beaglebones at run time. 
- Overlays: Adjust the state of the GPIOS to accomplished desired functionality at boot time. 
- Scripts: Variaty of functions. The main is creating a serial to Network proxy on both sides. This accomplishes backward compatibility, as the rocket legacy communication protocol is UART. 
