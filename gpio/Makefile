FLAGS =  -std=c11  -D_DEFAULT_SOURCE  -Werror -lm $(shell pkg-config --cflags --libs check)

GPI0_COMPILING =  main.c gpio_handler.c  -o build/gpio_test $(FLAGS)

gpio: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	@mkdir -p build
	@gcc  $(GPI0_COMPILING)
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST ***'
	@echo -e '*************************'
	@sudo ./build/gpio_test $(SERVER_PARAM)