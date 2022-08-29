#include "server_mgr.h"


int run_server(int myport_input, const char* certificate_input, const char* priv_key_input, const char* ca_cert){
    
    gpio_set_t led_set;

     memset(&led_set, 0 , sizeof(gpio_set_t));

    connection_params_t* params = malloc(sizeof(connection_params_t));
    MEMORY_ERROR( params);
    params->host_state = NOT_CONNECTED;
    params->host_port= myport_input;
    params->host_certificate = certificate_input;
    params->host_key = priv_key_input;
    params->host_is_client = FALSE;
    params->ca_cert= ca_cert;

    #ifdef NO_GPIO 
        printf("\n\nNo GPIOs are used\n\n");
    #else 
        printf("\n\nGPIOs are used\n\n");
    #endif
    
    ///Stars openssl library and allow server to receive new connections 
    if ((star_host_connection(params)==0)&&(start_port_server(params)==0)) {//TODO: is the error printed?
    start_gpio(&led_set);

        do{
            
            if (connect_with_client(params))printf("Attempt to connect with client fail");
            else{
                params->host_state = ZERO;
                ShowCerts(params->remote_ssl);
                select_state_server(params, &led_set);
                close_remote_conn(params);
            }
        }while(params->host_state !=SHUT_DOWN);
        close_server(params);
        close_gpio(&led_set);
        return 0;
    }
    return 1;
    
}

int select_state_server(connection_params_t* params,gpio_set_t* led_set ){
    wireless_data_t* msg_received = malloc(sizeof(connection_params_t));
    MEMORY_ERROR(msg_received);
    do{
         if(read_from_remote(params, msg_received))break;
        else{
             switch (msg_received->command)
            {
            case SHUTDOWN_CO:
                run_shut_down_server(params,msg_received);
                break;
            case ZERO_CO:
                run_zero(params,msg_received,"Server is connected to Mission Control :)\n",led_set);
                break;
            case ARM_CO:
                run_arm(params,msg_received,led_set);
                break;
            case IGNITE_CO:
                run_ignite(params,msg_received,led_set);
                break;
                
            default:
                run_zero(params,msg_received,"ERROR: The CMD received is not valid.",led_set);
                break;
            }
        }
        printf("Server has changed to state = %d\n", params->host_state);
    }while(params->host_state != SHUT_DOWN);
    free(msg_received);
}

int run_shut_down_server(connection_params_t* params,wireless_data_t* msg_received){
    params->host_state = SHUT_DOWN;
    msg_received->cmd_status=ACK;
    write_to_remote( params, *msg_received);
    return 0;
}

int run_zero(connection_params_t* params,wireless_data_t* msg_received, char* error,gpio_set_t* led_set){
    if(error!=NULL)printf("%s", error);
    params->host_state = ZERO;
    msg_received->cmd_status=ACK;
    reset_gpio(led_set);
    write_to_remote( params, *msg_received);
    return 0;
}

int run_arm(connection_params_t* params,wireless_data_t* msg_received,gpio_set_t* led_set){
    params->host_state = ARM;
    msg_received->cmd_status=ACK;
    set_gpio_arm(msg_received->instruction_code,led_set);
    write_to_remote( params, *msg_received);
    return 0;
}
int run_ignite(connection_params_t* params,wireless_data_t* msg_received,gpio_set_t* led_set){

    if ((params->host_state == ZERO )){
        msg_received->instruction_code= 0xFF;
        run_zero(params,msg_received,"Please send Arm, before igniting",led_set);
    }
    else{
        params->host_state = IGNITE;
        msg_received->cmd_status=ACK;
        set_gpio_ign(msg_received->instruction_code,led_set);
        write_to_remote( params, *msg_received); 
    }
    return 0;
}

int set_gpio_arm(u_int8_t arm_signal,gpio_set_t* led_set){
    
    if(arm_signal!=0){
        
        printf("\nARMsignal as received: %d\n",arm_signal);
        unsigned char bit1,bit2,bit3,bit4,bit5,bit6;

        bit1 = (arm_signal & FLAG_SIGNAL_6) ? 1 : 0; 
        bit2 = (arm_signal & FLAG_SIGNAL_5) ? 1 : 0;
        bit3 = (arm_signal & FLAG_SIGNAL_4) ? 1 : 0;
        bit4 = (arm_signal & FLAG_SIGNAL_3) ? 1 : 0;
        bit5 = (arm_signal & FLAG_SIGNAL_2) ? 1 : 0;
        bit6 = (arm_signal & FLAG_SIGNAL_1) ? 1 : 0;

        printf ("ARM signal porcessed:%d%d%d%d%d%d\n",bit6,bit5,bit4,bit3,bit2,bit1);

        #ifndef NO_GPIO
            libsoc_gpio_set_level(led_set->arm6_led, bit6);
            libsoc_gpio_set_level(led_set->arm5_led, bit5);
            libsoc_gpio_set_level(led_set->arm4_led, bit4);
            libsoc_gpio_set_level(led_set->arm3_led, bit3);
            libsoc_gpio_set_level(led_set->arm2_led, bit2);
            libsoc_gpio_set_level(led_set->arm1_led, bit1);
        #endif
    }
    return 0;
}
int set_gpio_ign(u_int8_t ign_signal,gpio_set_t* led_set){
    if(ign_signal!=0){
        printf("\nIGN signal as received: %d\n",ign_signal);
        unsigned char bit1,bit2,bit3,bit4,bit5,bit6;

        bit1 = (ign_signal & FLAG_SIGNAL_6) ? 1 : 0; 
        bit2 = (ign_signal & FLAG_SIGNAL_5) ? 1 : 0;
        bit3 = (ign_signal & FLAG_SIGNAL_4) ? 1 : 0;
        bit4 = (ign_signal & FLAG_SIGNAL_3) ? 1 : 0;
        bit5 = (ign_signal & FLAG_SIGNAL_2) ? 1 : 0;
        bit6 = (ign_signal & FLAG_SIGNAL_1) ? 1 : 0;

        printf ("IGN signal porcessed:%d%d%d%d%d%d\n",bit6,bit5,bit4,bit3,bit2,bit1);

        #ifndef NO_GPIO
            libsoc_gpio_set_level(led_set->con6_led, bit6);
            libsoc_gpio_set_level(led_set->con5_led, bit5);
            libsoc_gpio_set_level(led_set->con4_led, bit4);
            libsoc_gpio_set_level(led_set->con3_led, bit3);
            libsoc_gpio_set_level(led_set->con2_led, bit2);
            libsoc_gpio_set_level(led_set->con1_led, bit1);
        #endif

        return 0;
    }
    return 1;
}
int start_gpio( gpio_set_t* led_set){

    #ifndef NO_GPIO
        printf("Starting GPIO...\n");
        libsoc_set_debug(1);
        led_set->arm1_led = start__led_gpio(PIN_ARM_1);
        led_set->arm2_led = start__led_gpio(PIN_ARM_2);
        led_set->arm3_led = start__led_gpio(PIN_ARM_3);
        led_set->arm4_led = start__led_gpio(PIN_ARM_4);
        led_set->arm5_led = start__led_gpio(PIN_ARM_5);
        led_set->arm6_led = start__led_gpio(PIN_ARM_6);

        led_set->con1_led = start__led_gpio(PIN_IGN_1);
        led_set->con2_led = start__led_gpio(PIN_IGN_2);
        led_set->con3_led = start__led_gpio(PIN_IGN_3);
        led_set->con4_led = start__led_gpio(PIN_IGN_4);
        led_set->con5_led = start__led_gpio(PIN_IGN_5);
        led_set->con6_led = start__led_gpio(PIN_IGN_6);
    #else
        printf("Starting NNNNNo GPIO...\n");       
    #endif
    return 0;
}

int reset_gpio(gpio_set_t* led_set){
     #ifndef NO_GPIO 
        printf("GPIOs were reseted\n\n");
        libsoc_gpio_set_level(led_set->arm1_led, LOW);
        libsoc_gpio_set_level(led_set->arm2_led, LOW);
        libsoc_gpio_set_level(led_set->arm3_led, LOW);
        libsoc_gpio_set_level(led_set->arm4_led, LOW);
        libsoc_gpio_set_level(led_set->arm5_led, LOW);
        libsoc_gpio_set_level(led_set->arm6_led, LOW);

        libsoc_gpio_set_level(led_set->con1_led, LOW);
        libsoc_gpio_set_level(led_set->con2_led, LOW);
        libsoc_gpio_set_level(led_set->con3_led, LOW);
        libsoc_gpio_set_level(led_set->con4_led, LOW);
        libsoc_gpio_set_level(led_set->con5_led, LOW);
        libsoc_gpio_set_level(led_set->con6_led, LOW);
    #else 
        printf("NNNNNNNo GPIOs were reseted\n\n");
    #endif
}

int close_gpio(gpio_set_t* led_set){
    #ifndef NO_GPIO
        reset_gpio(led_set);
        int res = libsoc_gpio_free(led_set->arm1_led);
        if (res == EXIT_FAILURE)perror("Could not free gpio ");
        res = libsoc_gpio_free(led_set->arm2_led);
        if (res == EXIT_FAILURE)perror("Could not free gpio ");
        res = libsoc_gpio_free(led_set->arm3_led);
        if (res == EXIT_FAILURE)perror("Could not free gpio ");
        res = libsoc_gpio_free(led_set->arm4_led);
        if (res == EXIT_FAILURE)perror("Could not free gpio ");
        res = libsoc_gpio_free(led_set->arm5_led);
        if (res == EXIT_FAILURE)perror("Could not free gpio ");
        res = libsoc_gpio_free(led_set->arm6_led);
        if (res == EXIT_FAILURE)perror("Could not free gpio ");

        res = libsoc_gpio_free(led_set->con1_led);
        if (res == EXIT_FAILURE)perror("Could not free gpio ");
        res = libsoc_gpio_free(led_set->con2_led);
        if (res == EXIT_FAILURE)perror("Could not free gpio ");
        res = libsoc_gpio_free(led_set->con3_led);
        if (res == EXIT_FAILURE)perror("Could not free gpio ");
        res = libsoc_gpio_free(led_set->con4_led);
        if (res == EXIT_FAILURE)perror("Could not free gpio ");
        res = libsoc_gpio_free(led_set->con5_led);
        if (res == EXIT_FAILURE)perror("Could not free gpio ");
        res = libsoc_gpio_free(led_set->con6_led);
        if (res == EXIT_FAILURE)perror("Could not free gpio ");
        system("reboot");
    #endif
}
#ifndef NO_GPIO
gpio* start__led_gpio(int led_id){

    gpio* new_gpio = libsoc_gpio_request(led_id, LS_GPIO_SHARED);
	if (new_gpio == NULL ) printf("gpio were Unsuccessfully requested\n");
	libsoc_gpio_set_direction(new_gpio, OUTPUT);
	if (libsoc_gpio_get_direction(new_gpio) != OUTPUT)printf("Failed to set direction to INPUT\n");
    libsoc_gpio_set_level(new_gpio, LOW);
	if (libsoc_gpio_get_level(new_gpio) != LOW) printf("Failed setting gpio level LOW\n");

    return new_gpio;
}
#endif



int close_server(connection_params_t* params){
    close_host_conn(params);
    free( params);
    printf("\nServer program is shutting down..\n"); 
}