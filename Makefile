.DEFAULT_GOAL := non_gpio_client


FLAGS =  -std=c11  -D_DEFAULT_SOURCE  -Werror -lm $(shell pkg-config --cflags --libs check) -L/usr/local/lib -lssl -lcrypto 
SERVER_IP = 192.168.192.218
PORT = 85354
SERVER_PARAM = $(PORT) keys/server.crt keys/server_rsa_private.pem.unsecure keys/ca.crt 
CLIENT_PARAM = $(SERVER_IP) $(PORT) keys/client.crt keys/client_rsa_private.pem.unsecure keys/ca.crt 
FAKE_CLIENT = $(SERVER_IP) $(PORT) fake/fake_client.crt fake/fake_client_rsa_private.pem.unsecure
SERVER_COMPILING = server_mgr.c server_main.c conn_mgr.c  
CLIENT_COMPILING = conn_mgr.c client_main.c client_mgr.c 
GPI0_COMPILING =  gpio_handler.c 
DEBUG_FLAGS =  -DNO_GPIO



server: 
	@echo -e '\n******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  $(SERVER_COMPILING) $(GPI0_COMPILING) $(FLAGS) -lsoc -o build/rocket_server
	@echo -e '\n******************************'
	@echo -e '*** Running SERVER UNIT TEST ***'
	@echo -e '********************************'
	@sudo ./build/rocket_server $(SERVER_PARAM)

run_server:
	@echo -e '\n******************************'
	@echo -e '*** Running SERVER UNIT TEST ***'
	@echo -e '********************************'
	@sudo ./build/rocket_server $(SERVER_PARAM)



non_gpio_server:
	@echo -e '\n**********************************'
	@echo -e '*** Compiling for NON_GPIO_SERVER **'
	@echo -e '************************************'
	@mkdir -p build
	@gcc  $(SERVER_COMPILING) $(GPI0_COMPILING) $(DEBUG_FLAGS) $(FLAGS) -o build/no_gpio_rocket_server 
	@echo -e '\n******************************'
	@echo -e '*** Running NON_GPIO_SERVER  ***'
	@echo -e '********************************'
	@sudo ./build/no_gpio_rocket_server $(SERVER_PARAM)



client: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  $(CLIENT_COMPILING) $(FLAGS) -lsoc -o build/ignition_box
	@echo -e '\n*************************'
	@echo -e '*** Running CLIENT UNIT TEST ***'
	@echo -e '*************************'
	@sudo ./build/ignition_box $(CLIENT_PARAM) 

run_client:
	@echo -e '\n*************************'
	@echo -e '*** Running CLIENT UNIT TEST ***'
	@echo -e '*************************'
	@sudo ./build/ignition_box $(CLIENT_PARAM)


non_gpio_client: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  $(CLIENT_COMPILING) $(DEBUG_FLAGS) $(FLAGS) -o build/non_gpio_ignition_box
	@echo -e '\n*************************'
	@echo -e '*** Running CLIENT UNIT TEST ***'
	@echo -e '*************************'
	@sudo ./build/non_gpio_ignition_box $(CLIENT_PARAM)

 

fake_client: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  $(CLIENT_COMPILING) $(FLAGS)  -lsoc -o build/ignition_box
	@echo -e '\n*************************'
	@echo -e '*** Running FAKE CLIENT UNIT TEST ***'
	@echo -e '*************************'
	@sudo ./build/ignition_box $(FAKE_CLIENT) 

valgrind_server: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc -g  $(SERVER_COMPILING)  $(FLAGS)
	@echo -e '*** Running VALGRIND TEST ***'
	@echo -e '*************************'
	@valgrind  --show-leak-kinds=all ./build/server_test $(SERVER_PARAM)

valgrind_client: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc -g  $(CLIENT_COMPILING) $(FLAGS)
	@echo -e '*** Running VALGRIND TEST ***'
	@echo -e '*************************'
	@valgrind -s --leak-check=full ./build/server_test $(CLIENT_PARAM)


zip:
	zip lab7_ex2.zip connmgr.c  errmacros.h connmgr.h config.h lib/dplist.c lib/dplist.h lib/tcpsock.c lib/tcpsock.h
