#include "client_mgr.h"

int callback_con(void* arg)
{

    callback_params_t* args = arg;
	int btn= args->gpio_btn_c->gpio;
	
	if(CONNECT_BTN_GPIO  == btn)	printf("\nCONNECT PRESSED...%d\n",btn);
	else if(ARM_BTN_GPIO==btn)	printf("\nARM PRESSED...%d\n",btn);
    else if(IGN_BTN_GPIO  == btn)	printf("\nCONNECT PRESSED...%d\n",btn);
	else if(RESET_BTN_GPIO==btn)	printf("\nARM PRESSED...%d\n",btn);
	else if(SHUT_BTN_GPIO==btn)	printf("\nARM PRESSED...%d\n",btn);

	return EXIT_SUCCESS;
}


int run_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input){
    // TODO: start Gpios and Set all to zero;

    //int gpio_btn_ids[] = {CONNECT_BTN_GPIO,ARM_BTN_GPIO,IGN_BTN_GPIO,RESET_BTN_GPIO,SHUT_BTN_GPIO};
    btn_pressed_t btn;

    connection_params_t* params = start_client(ip_address_input,  myport_input, certificate_input, priv_key_input);
    if(params==NULL)return 1;


    gpio_set_t *  btn_gpio = start_gpios(params);
   

    do{
/*         printf("Press zero to connect with server.\n");
        scanf("%d",(int*) &btn);
        if(btn == BTN_CONNECT){
                if(connect_with_rocket(params,ZERO_CO,REQUESTED,0)==0){
                        do{
                            printf("\nPress a Button:\nBTN_CONNECT=0\nBTN_ZERO=1\nBTN_ARM=2\nBTN_IGNITE=3\nBTN_SHUT_DOWN_SERVER=4\nBTN_SHUT_DOWN_CLIENT=5\n");
                            scanf("%d",(int*) &btn);
                            select_state(btn,params);
                        }while(params->host_state != SHUT_DOWN);
                }
        } */
    }while(params->host_state != SHUT_DOWN);
    exit_client(params);  
}

gpio_set_t * start_gpios(connection_params_t* params){
    printf("Starting gpio\n");
    libsoc_set_debug(1);
    gpio_set_t* btn_gpios = malloc(sizeof(gpio_set_t));
    MEMORY_ERROR(btn_gpios);
    btn_gpios->con_gpio_btn = start_individual_btn_gpio(params,CONNECT_BTN_GPIO);
    btn_gpios->arm_gpio_btn = start_individual_btn_gpio(params,ARM_BTN_GPIO);
    btn_gpios->ign_gpio_btn = start_individual_btn_gpio(params,IGN_BTN_GPIO);
    btn_gpios->reset_gpio_btn = start_individual_btn_gpio(params,RESET_BTN_GPIO);
    btn_gpios->shut_gpio_btn = start_individual_btn_gpio(params,SHUT_BTN_GPIO);
    return btn_gpios;
}

gpio* start_individual_btn_gpio(connection_params_t* params,int  btn_id){

    gpio* new_gpio = libsoc_gpio_request(btn_id, LS_GPIO_GREEDY);

	if (new_gpio == NULL ) printf("gpio were Unsuccessfully requested\n");
	
	libsoc_gpio_set_direction(new_gpio, INPUT);
	if (libsoc_gpio_get_direction(new_gpio) != INPUT)printf("Failed to set direction to INPUT\n");

    libsoc_gpio_set_edge(new_gpio, RISING);
  	if (libsoc_gpio_get_edge(new_gpio) != RISING) printf("Failed to set edge to RISING\n");

    callback_params_t* c_params =  malloc(sizeof(c_params));
    c_params->gpio_btn_c = new_gpio;
    c_params->params_c = params; 

  	int res = libsoc_gpio_callback_interrupt(new_gpio, &callback_con, (void *) c_params);
	if (res == EXIT_FAILURE) perror("Failed to set gpio callback");
    free(c_params);
    return new_gpio;
}



int select_state(btn_pressed_t input,connection_params_t* params){

    /// TODO: switch statement 
    if(input == BTN_SHUT_DOWN_SERVER) execute_command(params,SHUTDOWN_CO,REQUESTED,0);
    else if (input == BTN_SHUT_DOWN_CLIENT) params->host_state = SHUT_DOWN;
    else if ((input == BTN_ZERO)||(params->host_state == IGNITE)) execute_command(params,ZERO_CO,REQUESTED,0); 
    else if((input == BTN_ARM))execute_command(params,ARM_CO,REQUESTED,ARM_SIGNAL);
    else if((input == BTN_IGNITE)){
        if(params->host_state == ARM)execute_command(params,IGNITE_CO,REQUESTED,IGNITE_SIGNAL);
        else printf("Rocket must be armed before IGNITE!\n");
    }
    else printf("wrong input I/O error.\n");
    return 0; 
}


int connect_with_rocket(connection_params_t* params,commands_t command,status_t cmd_status,u_int8_t  instruction_code){
    if (connect_with_server(params)==0 && ShowCerts(params->remote_ssl)==0){
        params->host_state=ZERO;
        ///GPIOS are at zero and client is connected to server.
        return 0;
        
    }
    else{
        printf("Error connecting with server, check network settings.\n");
        return 1;

    }
}


int execute_command(connection_params_t* params,commands_t command,status_t cmd_status,u_int8_t  instruction_code){
    wireless_data_t msg_send;
    msg_send.command = command;
    msg_send.cmd_status = cmd_status;
    msg_send.instruction_code = instruction_code;
    //TODO: swtich()
    if(!command_handshake(params,msg_send)){
        if(msg_send.command==ZERO_CO){
            params->host_state=ZERO;  
            printf("Client has changed to state = %d\n", params->host_state);

        }
        else if(msg_send.command==ARM_CO){
            params->host_state=ARM;  
            printf("Client has changed to state = %d\n", params->host_state);

        }
        else if(msg_send.command==IGNITE_CO){
            params->host_state=IGNITE;  
            printf("Client has changed to state = %d\n", params->host_state);

        }
        else if(msg_send.command==SHUTDOWN_CO){
            params->host_state=SHUT_DOWN;  
            printf("Client has changed to state = %d\n", params->host_state);

        }
        return 0;
            
    }
    else{
        params->host_state=SHUT_DOWN; 
        printf("EXIT ater error on command_handshake()\n");
    }
    return 0;
}


int command_handshake(connection_params_t* params, wireless_data_t msg_send){
    wireless_data_t* msg_received = malloc(sizeof(connection_params_t));
    MEMORY_ERROR(msg_received);

    if (write_to_remote(params,msg_send)){
        printf("Sending failed! Error code= %d. Error message is:'%s'\n", errno, strerror(errno));
        return 1;
    }
    if (read_from_remote(params,msg_received)!= 0){ 
        //TODO: Add to all if statements.
        //BUG use else if. Handle better the sequances
        printf("Message reception failed!Error code= %d. Error message is:'%s'\n", errno, strerror(errno));
        return 1;
    }
    
    if ((msg_received->command!=msg_send.command)&&(msg_received->cmd_status!=ACK)){
        printf("Error: wrong ASK packet received: commad: %d\ncmd_status: %d\n",(int)msg_received->command,(int)msg_received->cmd_status);
        return 1;
    }
    //TODO: free if error too
    free(msg_received);
    return 0;
}

connection_params_t*  start_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input){
    // Start parameters that are used in the connection.
    connection_params_t* params = malloc(sizeof(connection_params_t));
    MEMORY_ERROR( params);
    params->host_state = NOT_CONNECTED;
    params->remote_ip_addr =ip_address_input;
    params->host_port= myport_input;
    params->host_certificate = certificate_input;
    params->host_key = priv_key_input;
    params->host_is_client = TRUE;

    if (star_host_connection(params)){ /// This part only needs to be done once.
        printf("error starting host\n");
        return NULL;
    }
    return params;
}

int exit_client(connection_params_t* params){
    if(params->host_state != NOT_CONNECTED) close_host_conn(params);
    free( params);
    printf("\nprogram is shutting down..\n");
    return 0;
}




