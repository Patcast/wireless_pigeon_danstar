FLAGS =  -std=c11  -D_DEFAULT_SOURCE  -Werror -lm $(shell pkg-config --cflags --libs check) -L/usr/local/lib -lssl -lcrypto
SERVER_IP = 127.0.0.1
PORT = 8537
SERVER_PARAM = $(PORT) keys/server.crt keys/server_rsa_private.pem.unsecure
CLIENT_PARAM = $(SERVER_IP) $(PORT) keys/client.crt keys/client_rsa_private.pem.unsecure
FAKE_CLIENT = $(SERVER_IP) $(PORT) fake/fake_client.crt fake/fake_client_rsa_private.pem.unsecure
SERVER_COMPILING = server_mgr.c server_main.c conn_mgr.c -o build/server_test $(FLAGS)
CLIENT_COMPILING = conn_mgr.c client_main.c client_mgr.c -o build/client_test $(FLAGS)
GPI0_COMPILING =  gpio_handler.c 

server: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  $(SERVER_COMPILING) $(GPI0_COMPILING)
	@echo -e '\n*************************'
	@echo -e '*** Running SERVER UNIT TEST ***'
	@echo -e '*************************'
	@sudo ./build/server_test $(SERVER_PARAM)

client: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  $(CLIENT_COMPILING)
	@echo -e '\n*************************'
	@echo -e '*** Running CLIENT UNIT TEST ***'
	@echo -e '*************************'
	@sudo ./build/client_test $(CLIENT_PARAM) 

fake_client: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  $(CLIENT_COMPILING)
	@echo -e '\n*************************'
	@echo -e '*** Running FAKE CLIENT UNIT TEST ***'
	@echo -e '*************************'
	@sudo ./build/client_test $(FAKE_CLIENT) 


server_example: server_pigeon/server_tls.c 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  server_pigeon/server_tls.c -o build/server_test $(FLAGS)
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST ***'
	@echo -e '*************************'
	@sudo ./build/server_test $(PORT)

client_example: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  client_pigeon/client_tls.c -o build/client_test $(FLAGS)
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST ***'
	@echo -e '*************************'
	@sudo ./build/client_test $(SERVER_IP) $(PORT)

debug_server: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc -g  $(SERVER_COMPILING) 
	@echo -e '\n*************************'
	@echo -e '*** Running DEBUGGER ***'
	@echo -e '*************************'
	@gdb -q -tui ./build/server_test $(SERVER_PARAM)
valgrind_server: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc -g  $(SERVER_COMPILING) 
	@echo -e '*** Running VALGRIND TEST ***'
	@echo -e '*************************'
	@valgrind  --show-leak-kinds=all ./build/server_test $(SERVER_PARAM)

valgrind_client: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc -g  $(CLIENT_COMPILING) 
	@echo -e '*** Running VALGRIND TEST ***'
	@echo -e '*************************'
	@valgrind -s --leak-check=full ./build/server_test $(CLIENT_PARAM)


zip:
	zip lab7_ex2.zip connmgr.c  errmacros.h connmgr.h config.h lib/dplist.c lib/dplist.h lib/tcpsock.c lib/tcpsock.h
