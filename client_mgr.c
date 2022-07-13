#include "client_mgr.h"

int callback_con(void* arg)
{

    individual_callback_params_t* c_params = (individual_callback_params_t*) arg;
	int btn= c_params-> gpio_id;

    if(c_params->connection_params->host_state  > NOT_CONNECTED){
        // use select input. 
        select_state(btn,c_params->connection_params);
    }
    else if(btn== CONNECT_BTN_GPIO &&c_params->connection_params->host_state == NOT_CONNECTED){
        connect_with_rocket(c_params->connection_params,ZERO_CO,REQUESTED,0); //
    }
    else printf("Press CON to connect\n");
    sleep(1);
	return EXIT_SUCCESS;
}


int run_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input){
    // TODO: start Gpios and Set all to zero;

    //int gpio_btn_ids[] = {CONNECT_BTN_GPIO,ARM_BTN_GPIO,IGN_BTN_GPIO,RESET_BTN_GPIO,SHUT_BTN_GPIO};
    //btn_pressed_t btn;

    callback_params_t pt_call_params;

    connection_params_t* connection_params = start_client(ip_address_input,  myport_input, certificate_input, priv_key_input);
    if(connection_params ==NULL)return 1;

    
    gpio_set_t *  btn_gpios = start_gpios(connection_params,&pt_call_params); // remeber to free this 
    if (btn_gpios==NULL) printf("btn_gpios is null");

    do{
        sleep(1);
    }while(connection_params->host_state != SHUT_DOWN);
    close_client(connection_params,btn_gpios); 

    return 0; 
}

gpio_set_t * start_gpios(connection_params_t* pt_connection_params, callback_params_t* pt_call_params){

    printf("Starting gpio\n");
    libsoc_set_debug(1);
    gpio_set_t* btn_gpios = malloc(sizeof(gpio_set_t)); 
    MEMORY_ERROR(btn_gpios);
    
    (pt_call_params->con_callback).gpio_id = CONNECT_BTN_GPIO;
    (pt_call_params->con_callback).connection_params = pt_connection_params;
    (pt_call_params->arm_callback).gpio_id = ARM_BTN_GPIO;
    (pt_call_params->arm_callback).connection_params = pt_connection_params;
    (pt_call_params->ign_callback).gpio_id = IGN_BTN_GPIO;
    (pt_call_params->ign_callback).connection_params = pt_connection_params;
    (pt_call_params->reset_callback).gpio_id = RESET_BTN_GPIO;
    (pt_call_params->reset_callback).connection_params = pt_connection_params;
    (pt_call_params->shut_callback).gpio_id = SHUT_BTN_GPIO;
    (pt_call_params->shut_callback).connection_params = pt_connection_params;

    btn_gpios->con_gpio_btn = start_individual_btn_gpio(&(pt_call_params->con_callback));
    btn_gpios->arm_gpio_btn = start_individual_btn_gpio(&(pt_call_params->arm_callback));
    btn_gpios->ign_gpio_btn = start_individual_btn_gpio(&(pt_call_params->ign_callback));
    btn_gpios->reset_gpio_btn = start_individual_btn_gpio(&(pt_call_params->reset_callback));
    btn_gpios->shut_gpio_btn = start_individual_btn_gpio(&(pt_call_params->shut_callback));

    return btn_gpios;
}

gpio* start_individual_btn_gpio(individual_callback_params_t* pt_ind_call_params){

    gpio* new_gpio = libsoc_gpio_request(pt_ind_call_params->gpio_id, LS_GPIO_GREEDY);
	if (new_gpio == NULL ) printf("gpio were Unsuccessfully requested\n");
	

	libsoc_gpio_set_direction(new_gpio, INPUT);
	if (libsoc_gpio_get_direction(new_gpio) != INPUT)printf("Failed to set direction to INPUT\n");

    libsoc_gpio_set_edge(new_gpio, RISING);
  	if (libsoc_gpio_get_edge(new_gpio) != RISING) printf("Failed to set edge to RISING\n");


  	int res = libsoc_gpio_callback_interrupt(new_gpio, &callback_con, (void *) pt_ind_call_params);
	if (res == EXIT_FAILURE) perror("Failed to set gpio callback");
    return new_gpio;
}



int select_state(btn_pressed_t input,connection_params_t* params){

    /// TODO: switch statement 
    if(input == SHUT_BTN_GPIO) execute_command(params,SHUTDOWN_CO,REQUESTED,0);
    //else if (input == SHUT_BTN_GPIO) params->host_state = SHUT_DOWN;// function not used atm. in this case only the client is shut down. 
    else if ((input == RESET_BTN_GPIO)||(params->host_state == IGNITE)) execute_command(params,ZERO_CO,REQUESTED,0); 
    else if((input == ARM_BTN_GPIO))execute_command(params,ARM_CO,REQUESTED,ARM_SIGNAL);
    else if((input == IGN_BTN_GPIO)){
        if(params->host_state == ARM)execute_command(params,IGNITE_CO,REQUESTED,IGNITE_SIGNAL);
        else printf("Rocket must be armed before IGNITE!\n");
    }
    else if( input == CONNECT_BTN_GPIO) printf("Client is already connected\n");
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

int close_client(connection_params_t* params,gpio_set_t *  btn_gpios){
    if(params->host_state != NOT_CONNECTED) close_host_conn(params);
    free( params);
    free(btn_gpios);
    printf("\nprogram is shutting down..\n");
    return 0;
}




