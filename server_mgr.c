#include "server_mgr.h"

// TODO: Create non gpio compatible mode.
// TODO: Ensure that code modifies output of gpios
// TODO: Double check that the gpio work with the old methods, otherwise subtitude for client methods. 

//unsigned int PIN_ARM_1 = 86;

// BUG: PRevent cancel IGNITE signal due to debouncing. Redraw diagram. Also remove the function that If ignite is pressed twice it stops. 

int run_server(int myport_input, const char* certificate_input, const char* priv_key_input){

    connection_params_t* params = malloc(sizeof(connection_params_t));
    MEMORY_ERROR( params);
    params->host_state = NOT_CONNECTED;
    params->host_port= myport_input;
    params->host_certificate = certificate_input;
    params->host_key = priv_key_input;
    params->host_is_client = FALSE;

    #ifdef NO_GPIO 
        printf("\n\nNo GPIOs are used\n\n");
    #else 
        printf("\n\nGPIOs are used\n\n");
    #endif
    
    ///Stars openssl library and allow server to receive new connections 
    if ((star_host_connection(params)==0)&&(start_port_server(params)==0)) {//TODO: is the error printed?
    start_gpio();

        do{
            
            if (connect_with_client(params))printf("Attempt to connect with client fail");
            else{
                params->host_state = ZERO;
                ShowCerts(params->remote_ssl);
                //TODO: SET GPIO to zero.
                select_state_server(params);
                close_remote_conn(params);
            }
        }while(params->host_state !=SHUT_DOWN);
        close_server(params);
        close_gpio();
        return 0;
    }
    return 1;
    
}

int select_state_server(connection_params_t* params){
    wireless_data_t* msg_received = malloc(sizeof(connection_params_t));
    MEMORY_ERROR(msg_received);
    int bytes;

    do{
        //printf("Server is reading\n");
        //TODO: use switch statement.
        if(read_from_remote(params, msg_received))break;
        else{
            if (msg_received->command == SHUTDOWN_CO)run_shut_down_server(params,msg_received);
            else if (params->host_state == IGNITE)run_zero(params,msg_received,"ERROR:CMD called after ignite. Setting zero state again\n");//BUG: This might be inconvinient.
            else if ((msg_received->command == ZERO_CO ))run_zero(params,msg_received,"Server is connected to Mission Control :)\n");
            else if ((msg_received->command == ARM_CO ))run_arm(params,msg_received);
            else if ((msg_received->command == IGNITE_CO ))run_ignite(params,msg_received);
            else run_zero(params,msg_received,"ERROR: The CMD received is not valid.");
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

int run_zero(connection_params_t* params,wireless_data_t* msg_received, char* error){
    if(error!=NULL)printf("%s", error);
    params->host_state = ZERO;
    msg_received->cmd_status=ACK;
    reset_gpio();
    write_to_remote( params, *msg_received);
    return 0;
}

int run_arm(connection_params_t* params,wireless_data_t* msg_received){
    params->host_state = ARM;
    msg_received->cmd_status=ACK;
    set_gpio_arm(msg_received->instruction_code);
    write_to_remote( params, *msg_received);
    return 0;
}
int run_ignite(connection_params_t* params,wireless_data_t* msg_received){
    if ((params->host_state == ARM )){
        params->host_state = IGNITE;
        msg_received->cmd_status=ACK;
        set_gpio_ign(msg_received->instruction_code);
        write_to_remote( params, *msg_received);
    }
    else {
        msg_received->instruction_code= 0xFF;
        run_zero(params,msg_received,"Please send Arm, before igniting");
    }
    return 0;
}

int set_gpio_arm(u_int8_t arm_signal){
    
    if(arm_signal!=0){
        
        printf("\nARMsignal as received: %d\n",arm_signal);
        unsigned char bit1,bit2,bit3,bit4,bit5,bit6;

        bit6 = (arm_signal & FLAG_SIGNAL_6) ? 1 : 0; 
        bit5 = (arm_signal & FLAG_SIGNAL_5) ? 1 : 0;
        bit4 = (arm_signal & FLAG_SIGNAL_4) ? 1 : 0;
        bit3 = (arm_signal & FLAG_SIGNAL_3) ? 1 : 0;
        bit2 = (arm_signal & FLAG_SIGNAL_2) ? 1 : 0;
        bit1 = (arm_signal & FLAG_SIGNAL_1) ? 1 : 0;

        printf ("ARM signal porcessed:%d%d%d%d%d%d\n",bit6,bit5,bit4,bit3,bit2,bit1);

        #ifndef NO_GPIO
        gpio_set_value(PIN_ARM_6, bit6);
        gpio_set_value(PIN_ARM_5, bit5);
        gpio_set_value(PIN_ARM_4, bit4);
        gpio_set_value(PIN_ARM_3, bit3);
        gpio_set_value(PIN_ARM_2, bit2);
        gpio_set_value(PIN_ARM_1, bit1);
        #endif
    }
    return 0;
}
int set_gpio_ign(u_int8_t ign_signal){
    if(ign_signal!=0){
        printf("\nIGN signal as received: %d\n",ign_signal);
        unsigned char bit1,bit2,bit3,bit4,bit5,bit6;

        bit6 = (ign_signal & FLAG_SIGNAL_6) ? 1 : 0; 
        bit5 = (ign_signal & FLAG_SIGNAL_5) ? 1 : 0;
        bit4 = (ign_signal & FLAG_SIGNAL_4) ? 1 : 0;
        bit3 = (ign_signal & FLAG_SIGNAL_3) ? 1 : 0;
        bit2 = (ign_signal & FLAG_SIGNAL_2) ? 1 : 0;
        bit1 = (ign_signal & FLAG_SIGNAL_1) ? 1 : 0;

        printf ("IGN signal porcessed:%d%d%d%d%d%d\n",bit6,bit5,bit4,bit3,bit2,bit1);

        #ifndef NO_GPIO
        gpio_set_value(PIN_IGN_6, bit6);
        gpio_set_value(PIN_IGN_5, bit5);
        gpio_set_value(PIN_IGN_4, bit4);
        gpio_set_value(PIN_IGN_3, bit3);
        gpio_set_value(PIN_IGN_2, bit2);
        gpio_set_value(PIN_IGN_1, bit1);
        #endif

        return 0;
    }
    return 1;
}
int start_gpio(){

    // BUG : If gpio files already exist, this will crash.  Consider usng same approach as with the client.
    #ifndef NO_GPIO
        gpio_export(PIN_ARM_1);  
        gpio_set_dir(PIN_ARM_1, OUTPUT_PIN); 
        gpio_export(PIN_ARM_2);  
        gpio_set_dir(PIN_ARM_2, OUTPUT_PIN);
        gpio_export(PIN_ARM_3);  
        gpio_set_dir(PIN_ARM_3, OUTPUT_PIN);
        gpio_export(PIN_ARM_4);  
        gpio_set_dir(PIN_ARM_4, OUTPUT_PIN);
        gpio_export(PIN_ARM_5);  
        gpio_set_dir(PIN_ARM_5, OUTPUT_PIN);
        gpio_export(PIN_ARM_6);  
        gpio_set_dir(PIN_ARM_6, OUTPUT_PIN);

        gpio_export(PIN_IGN_1);  
        gpio_set_dir(PIN_IGN_1, OUTPUT_PIN); 
        gpio_export(PIN_IGN_2);  
        gpio_set_dir(PIN_IGN_2, OUTPUT_PIN);
        gpio_export(PIN_IGN_3);  
        gpio_set_dir(PIN_IGN_3, OUTPUT_PIN);
        gpio_export(PIN_IGN_4);  
        gpio_set_dir(PIN_IGN_4, OUTPUT_PIN);
        gpio_export(PIN_IGN_5);  
        gpio_set_dir(PIN_IGN_5, OUTPUT_PIN);
        gpio_export(PIN_IGN_6);  
        gpio_set_dir(PIN_IGN_6, OUTPUT_PIN); 
    #else
        printf("Starting GPIO...\n");       
    #endif
    return 0;
}

int reset_gpio(){
     #ifndef NO_GPIO 
        gpio_set_value(PIN_ARM_1, LOW);
        gpio_set_value(PIN_ARM_2, LOW);
        gpio_set_value(PIN_ARM_3, LOW);
        gpio_set_value(PIN_ARM_4, LOW);
        gpio_set_value(PIN_ARM_5, LOW);
        gpio_set_value(PIN_ARM_6, LOW);
        gpio_set_value(PIN_IGN_1, LOW);
        gpio_set_value(PIN_IGN_2, LOW);
        gpio_set_value(PIN_IGN_3, LOW);
        gpio_set_value(PIN_IGN_4, LOW);
        gpio_set_value(PIN_IGN_5, LOW);
        gpio_set_value(PIN_IGN_6, LOW);
    #else 
        printf("GPIOs were reseted\n\n");
    #endif
}

int close_gpio(){
    #ifndef NO_GPIO
        gpio_unexport(PIN_ARM_1);
        gpio_unexport(PIN_ARM_2);
        gpio_unexport(PIN_ARM_3);
        gpio_unexport(PIN_ARM_4);
        gpio_unexport(PIN_ARM_5);
        gpio_unexport(PIN_ARM_6);

        gpio_unexport(PIN_IGN_1);
        gpio_unexport(PIN_IGN_2);
        gpio_unexport(PIN_IGN_3);
        gpio_unexport(PIN_IGN_4);
        gpio_unexport(PIN_IGN_5);
        gpio_unexport(PIN_IGN_6);
    #endif
}



int close_server(connection_params_t* params){
    close_host_conn(params);
    free( params);
    printf("\nServer program is shutting down..\n"); 
}