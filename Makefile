FLAGS =  -std=c11  -D_DEFAULT_SOURCE  -Werror -lm $(shell pkg-config --cflags --libs check) -L/usr/local/lib -lssl -lcrypto
SERVER_IP = 127.0.0.1
PORT = 8069
SERVER_PARAM = $(PORT) 1 keys/server.crt keys/server_rsa_private.pem.unsecure
CLIENT_PARAM = $(SERVER_IP) $(PORT) keys/client.crt keys/client_rsa_private.pem.unsecure
FAKE_CLIENT = $(SERVER_IP) $(PORT) fake/fake_client.crt fake/fake_client_rsa_private.pem.unsecure


server: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  server_mgr.c main_server.c -o build/server_test $(FLAGS)
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST ***'
	@echo -e '*************************'
	@sudo ./build/server_test $(SERVER_PARAM)

client: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  client_mgr.c -o build/client_test $(FLAGS)
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST ***'
	@echo -e '*************************'
	@sudo ./build/client_test $(CLIENT_PARAM) 

fake_client: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  client_mgr.c -o build/client_test $(FLAGS)
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST ***'
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

debug: main.c sbuffer.c
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc main.c sbuffer.c   -lmylist -L/lib/ -g -o build/test $(FLAGS)
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST ***'
	@echo -e '*************************'
	@gdb -q -tui ./build/test
valgrind: main.c sbuffer.c
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc main.c sbuffer.c -lmylist -L/lib/ -g -o build/test $(FLAGS)
	@echo -e '*** Running UNIT TEST ***'
	@echo -e '*************************'
	@export CK_FORK=no ;  valgrind -s ./build/test
zip:
	zip lab7_ex2.zip connmgr.c  errmacros.h connmgr.h config.h lib/dplist.c lib/dplist.h lib/tcpsock.c lib/tcpsock.h