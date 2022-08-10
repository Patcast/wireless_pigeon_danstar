#include "client_mgr.h"

#ifndef NO_GPIO
int callback_con(void* arg)
{

    individual_callback_params_t* c_params = (individual_callback_params_t*) arg;
	int btn= c_params-> gpio_id;

    if(c_params->connection_params->host_state  > NOT_CONNECTED){
        // use select input. 
        select_state(btn,c_params);
    }
    else if(btn== CONNECT_BTN_GPIO){
        if(c_params->connection_params->host_state == NOT_CONNECTED){
            if(connect_with_rocket(c_params->connection_params,ZERO_CO,REQUESTED,0)==0)libsoc_gpio_set_level(c_params->pt_gpio_set->con_gpio_led, HIGH); //
        }
        else printf("MC alrady has a connection.\n");
        
    }
    //else if(btn == SHUT_BTN_GPIO && c_params->connection_params->host_state == NOT_CONNECTED ) c_params->connection_params->host_state = SHUT_DOWN; might be nice to close the program even if not connected. 
    else printf("Press CON to connect\n");
    sleep(1);
	return EXIT_SUCCESS;
}
#endif


int run_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input){

    callback_params_t pt_call_params;

    connection_params_t* conn_params = start_client(ip_address_input,  myport_input, certificate_input, priv_key_input);
    if(conn_params == NULL)return 1;
    
    #ifdef NO_GPIO 
        printf("\n\nNo GPIOs are used\n\n");
        btn_pressed_t btn;
        individual_callback_params_t c_params;    
        c_params.connection_params = conn_params;
        do{
            printf("Press zero to connect with server.\n");
            scanf("%d",(int*) &btn);
            if(btn == BTN_CONNECT){
                    if(connect_with_rocket(conn_params,ZERO_CO,REQUESTED,0)==0){
                            do{
                                printf("\nType the code of the button:\nBTN_CONNECT=\t30\nBTN_RESET=\t115\nBTN_ARM=\t31\nBTN_IGNITE=\t3\nBTN_SHUT_DOWN=\t49\n");
                                scanf("%d",(int*) &btn);
                                select_state(btn,&c_params);
                            }while(conn_params->host_state != SHUT_DOWN);
                    }
            }
        }while(conn_params->host_state != SHUT_DOWN);
    #else 
        printf("\n\nGPIOs are used\n\n");
        gpio_set_t *  btn_gpios = start_gpios(conn_params,&pt_call_params);
        if (btn_gpios==NULL) printf("btn_gpios is null");
        do{
            sleep(1);
        }while(conn_params->host_state != SHUT_DOWN);
        close_gpio(btn_gpios);
    #endif

    close_client(conn_params); 

    return 0; 
}




int select_state(btn_pressed_t input,individual_callback_params_t *  c_params){
    switch (input)
    {
    case SHUT_BTN_GPIO:
        execute_command(c_params,SHUTDOWN_CO,REQUESTED,0);
        break;
    case RESET_BTN_GPIO:
        execute_command(c_params,ZERO_CO,REQUESTED,0); 
        break;
    case ARM_BTN_GPIO:
        execute_command(c_params,ARM_CO,REQUESTED,ARM_SIGNAL);
        break;
    case IGN_BTN_GPIO:
        if(c_params->connection_params->host_state == ZERO)printf("Rocket must be armed before IGNITE!\n");
        else execute_command(c_params,IGNITE_CO,REQUESTED,IGNITE_SIGNAL);
        break;
    case CONNECT_BTN_GPIO:
         printf("Client is already connected\n");
        break;
    default:
        printf("wrong input I/O error.\n");
        break;
    }
    return 0; 
}


int connect_with_rocket(connection_params_t* params,commands_t command,status_t cmd_status,u_int8_t  instruction_code){
    if (connect_with_server(params)==0 && ShowCerts(params->remote_ssl)==0){
        params->host_state=ZERO;
        return 0; 
    }
    else{
        printf("Error connecting with server, check network settings.\n");
        return 1;
    }
}


int execute_command(individual_callback_params_t *  c_params,commands_t command,status_t cmd_status,u_int8_t  instruction_code){
    wireless_data_t msg_send;
    msg_send.command = command;
    msg_send.cmd_status = cmd_status;
    msg_send.instruction_code = instruction_code;

    if(!command_handshake(c_params->connection_params,msg_send)){
        switch (msg_send.command)
        {
            case ZERO_CO:
                c_params->connection_params->host_state=ZERO;
                #ifndef NO_GPIO
                    libsoc_gpio_set_level(c_params->pt_gpio_set->arm_gpio_led, LOW);
                    libsoc_gpio_set_level(c_params->pt_gpio_set->ign_gpio_led, LOW);
                    int i;
                    for(i=0; i<4; i++) 
                    {
                        if (i%2==0)libsoc_gpio_set_level(c_params->pt_gpio_set->reset_gpio_led, HIGH);
                        else libsoc_gpio_set_level(c_params->pt_gpio_set->reset_gpio_led, LOW);
                        sleep(1);
                    }
                #endif
                printf("Client has changed to state = %d\n", c_params->connection_params->host_state);
                break;
            case ARM_CO:
                c_params->connection_params->host_state=ARM;  
                printf("Client has changed to state = %d\n", c_params->connection_params->host_state);
                #ifndef NO_GPIO
                libsoc_gpio_set_level(c_params->pt_gpio_set->arm_gpio_led, HIGH);
                #endif
                break;
            case IGNITE_CO:
                c_params->connection_params->host_state=IGNITE;  
                printf("Client has changed to state = %d\n", c_params->connection_params->host_state);
                #ifndef NO_GPIO
                libsoc_gpio_set_level(c_params->pt_gpio_set->ign_gpio_led, HIGH);
                #endif
                break;
            case SHUTDOWN_CO:
                c_params->connection_params->host_state=SHUT_DOWN;  
                printf("Client has changed to state = %d\n", c_params->connection_params->host_state);
                break;
            default:
                printf("Unkown command. No action was executed.");
                break;
        }
        return 0;
            
    }
    else{
        c_params->connection_params->host_state=SHUT_DOWN; 
        printf("EXIT due to  error on command_handshake()\n");
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

int close_client(connection_params_t* params){
    if(params->host_state != NOT_CONNECTED) close_host_conn(params);
    free( params);
    printf("\nprogram is shutting down..\n");
    return 0;
}



/// **********************
/// GPIO RELETED METHODS
///**************************


#ifndef NO_GPIO
gpio_set_t * start_gpios(connection_params_t* pt_connection_params, callback_params_t* pt_call_params){

    printf("Starting gpio\n");
    //libsoc_set_debug(1);
    gpio_set_t* btn_gpios = malloc(sizeof(gpio_set_t)); 
    MEMORY_ERROR(btn_gpios);

    btn_gpios->con_gpio_led = start__led_gpio(LED_CON);
    btn_gpios->arm_gpio_led = start__led_gpio(LED_ARM);
    btn_gpios->ign_gpio_led = start__led_gpio(LED_IGN);
    btn_gpios->reset_gpio_led = start__led_gpio(LED_RESET);
    
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

    (pt_call_params->con_callback).pt_gpio_set = btn_gpios;
    (pt_call_params->arm_callback).pt_gpio_set = btn_gpios;
    (pt_call_params->ign_callback).pt_gpio_set = btn_gpios;
    (pt_call_params->reset_callback).pt_gpio_set = btn_gpios;
    (pt_call_params->shut_callback).pt_gpio_set = btn_gpios;
   

    btn_gpios->con_gpio_btn = start_individual_btn_gpio(&(pt_call_params->con_callback));
    btn_gpios->arm_gpio_btn = start_individual_btn_gpio(&(pt_call_params->arm_callback));
    btn_gpios->ign_gpio_btn = start_individual_btn_gpio(&(pt_call_params->ign_callback));
    btn_gpios->reset_gpio_btn = start_individual_btn_gpio(&(pt_call_params->reset_callback));
    btn_gpios->shut_gpio_btn = start_individual_btn_gpio(&(pt_call_params->shut_callback));

    return btn_gpios;
}

gpio* start__led_gpio(int led_id){

    gpio* new_gpio = libsoc_gpio_request(led_id, LS_GPIO_SHARED);
	if (new_gpio == NULL ) printf("gpio were Unsuccessfully requested\n");
	libsoc_gpio_set_direction(new_gpio, OUTPUT);
	if (libsoc_gpio_get_direction(new_gpio) != OUTPUT)printf("Failed to set direction to INPUT\n");
    libsoc_gpio_set_level(new_gpio, LOW);
	if (libsoc_gpio_get_level(new_gpio) != LOW) printf("Failed setting gpio level HIGH\n");

    return new_gpio;
}

gpio* start_individual_btn_gpio(individual_callback_params_t* pt_ind_call_params){

    gpio* new_gpio = libsoc_gpio_request(pt_ind_call_params->gpio_id, LS_GPIO_SHARED);
	if (new_gpio == NULL ) printf("gpio were Unsuccessfully requested\n");
	libsoc_gpio_set_direction(new_gpio, INPUT);
	if (libsoc_gpio_get_direction(new_gpio) != INPUT)printf("Failed to set direction to INPUT\n");

    libsoc_gpio_set_edge(new_gpio, RISING);
  	if (libsoc_gpio_get_edge(new_gpio) != RISING) printf("Failed to set edge to RISING\n");


  	int res = libsoc_gpio_callback_interrupt(new_gpio, &callback_con, (void *) pt_ind_call_params);
	if (res == EXIT_FAILURE) perror("Failed to set gpio callback");
    return new_gpio;
}


int close_gpio(gpio_set_t *  btn_gpios){

    libsoc_gpio_set_level(btn_gpios->con_gpio_led, LOW);
    libsoc_gpio_set_level(btn_gpios->arm_gpio_led, LOW);
    libsoc_gpio_set_level(btn_gpios->ign_gpio_led, LOW);

    int res = libsoc_gpio_free(btn_gpios->con_gpio_btn);
    if (res == EXIT_FAILURE)perror("Could not free gpio CON");
    res = libsoc_gpio_free(btn_gpios->shut_gpio_btn);
    if (res == EXIT_FAILURE)perror("Could not free gpio ARM");
    res = libsoc_gpio_free(btn_gpios->arm_gpio_btn);
    if (res == EXIT_FAILURE)perror("Could not free gpio IGN");
    res = libsoc_gpio_free(btn_gpios->ign_gpio_btn);
    if (res == EXIT_FAILURE)perror("Could not free gpio RESET");
    res = libsoc_gpio_free(btn_gpios->reset_gpio_btn);
    if (res == EXIT_FAILURE)perror("Could not free gpio SHUT");

     res = libsoc_gpio_free(btn_gpios->con_gpio_led);
    if (res == EXIT_FAILURE)perror("Could not free gpio CON");
    res = libsoc_gpio_free(btn_gpios->arm_gpio_led);
    if (res == EXIT_FAILURE)perror("Could not free gpio ARM");
    res = libsoc_gpio_free(btn_gpios->ign_gpio_led);
    if (res == EXIT_FAILURE)perror("Could not free gpio IGN");
    res = libsoc_gpio_free(btn_gpios->reset_gpio_led);
    if (res == EXIT_FAILURE)perror("Could not free gpio RESET");


    free(btn_gpios); 

    printf("\ngpios are closed..\n");

    return 0;
}
#endif




